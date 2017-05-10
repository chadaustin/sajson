import Foundation

public enum AllocationStrategy {
    case single
    case dynamic
}

/// Keep this in sync with sajson::type
private enum RawValueType: UInt8 {
    case integer = 0
    case double = 1
    case null = 2
    case bfalse = 3
    case btrue = 4
    case string = 5

    // TODO: Consider adding associated values here to enable basic operations on arrays and objects without
    // converting fully to a swift value.
    case array = 6
    case object = 7
}

/// Similar to `RawValueType`, but contains an associated (copied) swift type. This may deviate from `RawValueType`
/// for convenience (like containing a single boolean value).
public enum SwiftValuePayload {
    case integer(Int32)
    case double(Float64)
    case null
    case bool(Bool)
    case string(String)
    case array(ArrayReader)
    case object(ObjectReader)
}

extension SwiftValuePayload {
    public var string: String? {
        if case .string(let value) = self {
            return value
        }
        return nil
    }

    public var array: ArrayReader? {
        if case .array(let value) = self {
            return value
        }
        return nil
    }

    public var object: ObjectReader? {
        if case .object(let value) = self {
            return value
        }
        return nil
    }
}

// Encapsulates logic required to read from an array.
public struct ArrayReader {
    fileprivate init(payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.payload = payload
        self.input = input
    }

    public subscript(i: Int)-> SwiftValuePayload {
        let element = payload[1 + i]
        let elementType = RawValueType(rawValue: UInt8(element & 7))!
        let elementOffset = Int(element >> 3)
        return Value(type: elementType, payload: payload.advanced(by: elementOffset), input: input).swiftValue
    }

    public var count: Int {
        return Int(payload[0])
    }

    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

// Encapsulates logic required to read from an object.
public struct ObjectReader {
    fileprivate init(payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.payload = payload
        self.input = input
    }

    public subscript(key: String)-> SwiftValuePayload? {
        let parentValue = sajson_create_value(Int(RawValueType.object.rawValue), payload, input.baseAddress)

        let valuePtr = sajson_object_get_value_of_key(parentValue, key, key.lengthOfBytes(using: .utf8))
        guard let valuePtrUnwrapped = valuePtr else {
            return nil
        }

        let type = RawValueType(rawValue: sajson_get_value_type(valuePtrUnwrapped))!
        let payloadPtr = sajson_get_value_payload(valuePtrUnwrapped)!

        return Value(type: type, payload: payloadPtr, input: input).swiftValue
    }

    public var count: Int {
        return Int(payload[0])
    }

    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

public struct Value {
    fileprivate init(type: RawValueType, payload: UnsafePointer<UInt>, input: UnsafeBufferPointer<UInt8>) {
        self.type = type
        self.payload = payload
        self.input = input
    }

    // Recurses through any sub-values, and returns a copy using swift primitives.
    public var swiftValue: SwiftValuePayload {
        switch type {
        case .integer:
            return payload.withMemoryRebound(to: Int32.self, capacity: 1) { p in
                return SwiftValuePayload.integer(p[0])
            }
        case .double:
            let lo = UInt64(payload[0])
            let hi = UInt64(payload[1])
            let bitPattern = lo | (hi << 32)
            return .double(Float64(bitPattern: bitPattern))
        case .null:
            return .null
        case .bfalse:
            return .bool(false)
        case .btrue:
            return .bool(true)
        case .string:
            let start = Int(payload[0])
            let end = Int(payload[1])
            // TODO: are the following two lines a single copy?
            let data = Data(input[start ..< end])
            return .string(String(data: data, encoding: .utf8)!)
        case .array:
            return .array(ArrayReader(payload: payload, input: input))
        case .object:
            return .object(ObjectReader(payload: payload, input: input))
        }
    }


    // MARK: Private

    private let type: RawValueType
    private let payload: UnsafePointer<UInt>
    private let input: UnsafeBufferPointer<UInt8>
}

public final class Document {
    internal init(doc: OpaquePointer!) {
        self.doc = doc

        let rootType = RawValueType(rawValue: sajson_get_root_type(doc))!
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

    public lazy var swiftValue: SwiftValuePayload = { [weak self] in
        return self?.rootValue.swiftValue ?? .null
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
