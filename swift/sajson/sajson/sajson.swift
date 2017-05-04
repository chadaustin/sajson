public enum AllocationStrategy {
    case single
    case dynamic
}

public struct Value {
    public enum Type {
        case null
        case bfalse
        case btrue
        case string
        case number
        case array
        case object
    }
    
    

    let document: Document
}

public final class Document {
    // unsafe ptr to bytes, own
}

public func parse(allocationStrategy: AllocationStrategy, input: String) -> Document {
}
