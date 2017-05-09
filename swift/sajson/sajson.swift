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

    public static func fromOpaquePointer(_ ptr: OpaquePointer) -> RawValueType {
        return RawValueType(rawValue: sajson_get_value_type(ptr))!
    }
}

/// Similar to `RawValueType`, but contains an associated (copied) swift type. This may deviate from `RawValueType`
/// for convenience (like containing a single boolean value).
public enum SwiftValuePayload {
    case integer(Int32)
    case double(Float64)
    case null
    case bool(Bool)
    case string(String)
    case array([SwiftValuePayload])
    case object([String: SwiftValuePayload])
}

public struct Value {
    fileprivate init(type: RawValueType, ptr: OpaquePointer) {
        self.type = type
        self.ptr = ptr
    }

    // Recurses through any sub-values, and returns a copy using swift primitives.
    public var swiftValue: SwiftValuePayload {
        switch type {
        case .integer:
            return .integer(sajson_value_get_integer_value(ptr))
        case .double:
            return .double(sajson_value_get_double_value(ptr))
        case .null:
            return .null
        case .bfalse:
            return .bool(false)
        case .btrue:
            return .bool(true)
        case .string:
            return .string(unownedCStringToSwift(sajson_value_get_string_value(ptr)!))
        case .array:
            let length = sajson_value_get_length(ptr)
            var result = [SwiftValuePayload]()
            result.reserveCapacity(length)

            for i in 0..<length {
                let elementPtr = sajson_value_get_array_element(ptr, i)!
                let elementType = RawValueType.fromOpaquePointer(elementPtr)
                result.append(Value(type: elementType, ptr: elementPtr).swiftValue)
            }

            return .array(result)

        case .object:
            let length = sajson_value_get_length(ptr)

            var result = [String: SwiftValuePayload](minimumCapacity: length)
            for i in 0..<length {
                let key = unownedCStringToSwift(sajson_value_get_object_key(ptr, i)!)
                let elementPtr = sajson_value_get_object_value(ptr, i)!
                let elementType = RawValueType.fromOpaquePointer(elementPtr)
                result[key] = Value(type: elementType, ptr: elementPtr).swiftValue
            }

            return .object(result)
        }
    }


    // MARK: Private

    private let type: RawValueType
    private let ptr: OpaquePointer
}

public final class Document {
    internal init(doc: OpaquePointer!) {
        self.doc = doc

        let rootType = RawValueType(rawValue: sajson_get_root_type(doc))!
        let rootValuePointer = sajson_get_root(doc)!

        self.rootValue = Value(
            type: rootType,
            ptr: rootValuePointer)
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
    
    if sajson_has_error(dptr) {
        let errorString = unownedCStringToSwift(sajson_get_error_message(dptr)!)

        throw ParseError(
            line: sajson_get_error_line(dptr),
            column: sajson_get_error_column(dptr),
            message: errorString)
    }

    return Document(doc: dptr)
}

public func parse(allocationStrategy: AllocationStrategy, input: String) throws -> Document {
    return try parse(allocationStrategy: allocationStrategy, input: input.data(using: .utf8)!)
}

private func unownedCStringToSwift(_ cString: UnsafeMutablePointer<Int8>) -> String {
    let str = String(cString: cString)
    cString.deallocate(capacity: Int(strlen(cString)))
    return str
}
