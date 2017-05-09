import XCTest
import sajson_swift

class sajsonTests: XCTestCase {
    func test_empty_array() {
        _ = try! parse(allocationStrategy: .single, input: "[]")
    }

    func test_array() {
        let doc = try! parse(allocationStrategy: .single, input: "[10, \"Hello\"]")
        let docValue = doc.swiftValue

        guard case .array(let array) = docValue else { XCTFail(); return }
        XCTAssert(array.count == 2)
        guard case .integer(10) = array[0] else { XCTFail(); return }

        guard case .string("Hello") = array[1] else { XCTFail(); return }
    }

    func test_object() {
        let doc = try! parse(allocationStrategy: .single, input: "{\"hello\": \"world\"}")
        let docValue = doc.swiftValue

        guard case .object(let object) = docValue else { XCTFail(); return }
        XCTAssert(object.count == 1)
        guard case .some(.string("world")) = object["hello"] else { XCTFail(); return }
    }
}
