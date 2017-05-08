import XCTest
@testable import sajson_swift

class sajsonTests: XCTestCase {
    func test_empty_array() {
        _ = try! parse(allocationStrategy: .single, input: "[]")
    }
}
