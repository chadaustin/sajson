import XCTest
@testable import sajson

class sajsonTests: XCTestCase {
    func test_empty_array() {
        let doc = try! parse(allocationStrategy: .single, input: "[]")
        XCTAssertEqual(.array, doc.root.type)
        
        // This is an example of a functional test case.
        // Use XCTAssert and related functions to verify your tests produce the correct results.
    }
}
