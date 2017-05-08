import Foundation

public enum AllocationStrategy {
    case single
    case dynamic
}

// Keep this in sync with sajson::type
public enum RawValueType: UInt8 {
    case integer
    case double
    case null
    case bfalse
    case btrue
    case string
    case array
    case object
}

public struct Value {
    internal init(type: UInt8, ptr: UnsafePointer<UInt>) {
        self.type = type
        self.ptr = ptr
    }

    public let type: UInt8
    private let ptr: UnsafePointer<UInt>
}

public final class Document {
    internal init(doc: OpaquePointer!) {
        self.doc = doc
    }
    
    deinit {
        sajson_free_document(doc)
    }

    /// WARNING: Do not access the passed Value or any of its children
    /// outside of the callback.  The backing memory could be deallocated,
    /// causing use of the Value to cause a segfault.
    public func withRootValue<T>(_ cb: (_ root: Value) -> T) -> T {
        let rootType = sajson_get_root_type(doc)
        let rootPointer = sajson_get_root(doc)!
        
        return cb(Value(
            type: rootType,
            ptr: rootPointer))
    }
    
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
