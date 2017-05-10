import Foundation
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

    // MARK: Benchmarks

    func test_large_json_benchmark_parse_only() {
        let largeJsonData = createLargeTestJsonData(objectCount: 1000)

        measure {
            _ = try! parse(allocationStrategy: .single, input: largeJsonData)
        }
    }


    func test_large_json_benchmark_all() {
        let largeJsonData = createLargeTestJsonData(objectCount: 1000)

        measure {
            let doc = try! parse(allocationStrategy: .single, input: largeJsonData)
            let swiftValue = doc.swiftValue

            guard case .array(let array) = swiftValue else {
                preconditionFailure()
            }

            // Verify that something was actually deserialized.
            XCTAssert(array.count == 1000)
        }
    }

    // MARK: Helpers

    func createLargeTestJsonData(objectCount: Int) -> Data {
        var largeArray = [[String: Any]]()
        for _ in 0..<objectCount {
            largeArray.append(createTestJsonObject())
        }
        return try! JSONSerialization.data(withJSONObject: largeArray)
    }

    func createTestJsonObject() -> [String: Any] {
        var jsonDict =  [String: Any]()
        for i in 0..<100 {
            jsonDict["\(i)"] = randomString()
        }
        for i in 100..<200 {
            jsonDict["\(i)"] = randomInt()
        }
        return jsonDict
    }

    private func randomString() -> String {
        return UUID().uuidString
    }

    private func randomInt() -> Int32 {
        return Int32(arc4random_uniform(UInt32(Int32.max)))
    }
}
