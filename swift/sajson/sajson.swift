import Foundation

public enum AllocationStrategy {
    case single
    case dynamic
}

// Keep this in sync with sajson::type
private struct RawValueType {
    static let integer: UInt8 = 0
    static let double: UInt8 = 1
    static let null: UInt8 = 2
    static let bfalse: UInt8 = 3
    static let btrue: UInt8 = 4
    static let string: UInt8 = 5
    static let array: UInt8 = 6
    static let object: UInt8 = 7
}

/// Represents a JSON value decoded from the sajson AST.  This type provides decoded access
/// to array and object elements lazily.
public enum ShallowValue {
    case integer(Int32)
    case double(Float64)
    case null
    case bool(Bool)
    case string(String)
    case array(ArrayReader)
    case object(ObjectReader)
}

// Encapsulates logic required to read from an array.
public struct ArrayReader: Sequence {
    fileprivate init(payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.payload = payload
        self.input = input
    }

    public subscript(i: Int)-> ShallowValue {
        if i >= count {
            preconditionFailure("Index out of range: \(i)")
        }

        let element = payload[1 + i]
        let elementType = UInt8(element & 7)
        let elementOffset = Int(element >> 3)
        return Value(type: elementType, payload: payload.advanced(by: elementOffset), input: input).shallowValue
    }

    public var count: Int {
        return Int(payload[0])
    }

    // MARK: Sequence

    public struct Iterator: IteratorProtocol {
        fileprivate init(arrayReader: ArrayReader) {
            self.arrayReader = arrayReader
        }

        public mutating func next() -> ShallowValue? {
            if currentIndex < arrayReader.count {
                let value = arrayReader[currentIndex]
                currentIndex += 1
                return value
            } else {
                return nil
            }
        }

        private var currentIndex = 0
        private let arrayReader: ArrayReader
    }

    public func makeIterator() -> ArrayReader.Iterator {
        return Iterator(arrayReader: self)
    }

    // MARK: Private

    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

// Encapsulates logic required to read from an object.
public struct ObjectReader {
    fileprivate init(payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.payload = payload
        self.input = input
    }

    public subscript(key: String) -> ShallowValue? {
        let objectLocation = sajson_find_object_key(payload, key, key.lengthOfBytes(using: .utf8), input.baseAddress!)
        if objectLocation >= count {
            return nil
        }

        let element = payload[3 + objectLocation * 3]
        let elementType = UInt8(element & 7)
        let elementOffset = Int(element >> 3)
        return Value(type: elementType, payload: payload.advanced(by: elementOffset), input: input).shallowValue
    }

    public var count: Int {
        return Int(payload[0])
    }

    /// Returns the object as a dictionary. Should generally be avoided, as it is less efficient than directly reading
    /// values.
    public func asDictionary() -> [String: ShallowValue] {
        var result = [String: ShallowValue](minimumCapacity: self.count)
        for i in 0..<self.count {
            let start = Int(payload[1 + i * 3])
            let end = Int(payload[2 + i * 3])
            let value = Int(payload[3 + i * 3])

            let data = Data(input[start ..< end])
            let key = String(data: data, encoding: .utf8)!

            let valueType = UInt8(value & 7)
            let valueOffset = Int(value >> 3)
            result[key] = Value(type: valueType, payload: payload.advanced(by: valueOffset), input: input).shallowValue
        }
        return result
    }

    // MARK: Private

    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

private struct Value {
    fileprivate init(type: UInt8, payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.type = type
        self.payload = payload
        self.input = input
    }

    public var shallowValue: ShallowValue {
        switch type {
        case RawValueType.integer:
            return payload.withMemoryRebound(to: Int32.self, capacity: 1) { p in
                return .integer(p[0])
            }
        case RawValueType.double:
            let lo = UInt64(payload[0])
            let hi = UInt64(payload[1])
            let bitPattern = lo | (hi << 32)
            return .double(Float64(bitPattern: bitPattern))
        case RawValueType.null:
            return .null
        case RawValueType.bfalse:
            return .bool(false)
        case RawValueType.btrue:
            return .bool(true)
        case RawValueType.string:
            let start = Int(payload[0])
            let end = Int(payload[1])
            // TODO: are the following two lines a single copy?
            let data = Data(input[start ..< end])
            return .string(String(data: data, encoding: .utf8)!)
        case RawValueType.array:
            return .array(ArrayReader(payload: payload, input: input))
        case RawValueType.object:
            return .object(ObjectReader(payload: payload, input: input))
        default:
            fatalError("Unknown sajson value type - memory corruption?")
        }
    }


    // MARK: Private

    private let type: UInt8
    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

public final class Document {
    internal init(doc: OpaquePointer!) {
        self.doc = doc

        let rootType = sajson_get_root_type(doc)
        let rootValuePaylod = sajson_get_root(doc)!
        let inputPointer = sajson_get_input(doc)!
        let inputLength = sajson_get_input_length(doc)

        self.rootValue = Value(
            type: rootType,
            payload: rootValuePaylod,
            input: UnsafeBufferPointer(start: inputPointer, count: inputLength))
    }
    
    deinit {
        sajson_free_document(doc)
    }

    public lazy var swiftValue: ShallowValue = { [weak self] in
        return self?.rootValue.shallowValue ?? .null
    }()

    // MARK: Private

    /// TODO: The document's full swift representation/copy isn't exposed until we can figure out how to ensure memory
    /// associated with each value sticks around. Until then, all of the JSON is parsed at once and available through
    /// the `swiftValue` field above.
    private let rootValue: Value
    
    private let doc: OpaquePointer!
}

public final class ParseError: Error {
    internal init(line: Int, column: Int, message: String) {
        self.line = line
        self.column = column
        self.message = message
    }
    
    public let line: Int
    public let column: Int
    public let message: String
}

public func parse(allocationStrategy: AllocationStrategy, input: Data) throws -> Document {
    var copy = input // TODO: figure out if this is actually necessary
    let dptr: OpaquePointer! = copy.withUnsafeMutableBytes { (ptr: UnsafeMutablePointer<Int8>) in
        switch allocationStrategy {
        case .single:
            return sajson_parse_single_allocation(ptr, input.count)
        case .dynamic:
            return sajson_parse_dynamic_allocation(ptr, input.count)
        }
    }
    
    if dptr == nil {
        fatalError("Out of memory: failed to allocate document structure")
    }
    
    if sajson_has_error(dptr) != 0 {
        throw ParseError(
            line: sajson_get_error_line(dptr),
            column: sajson_get_error_column(dptr),
            message: String(cString: sajson_get_error_message(dptr)))
    }

    return Document(doc: dptr)
}

public func parse(allocationStrategy: AllocationStrategy, input: String) throws -> Document {
    return try parse(allocationStrategy: allocationStrategy, input: input.data(using: .utf8)!)
}
