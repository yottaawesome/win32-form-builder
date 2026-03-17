#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

// === FormException basics ===

TEST_CASE("FormException derives from std::runtime_error", "[errors]")
{
	auto ex = FormException(FormErrorCode::ParseError, "test message");
	const std::runtime_error& base = ex;
	REQUIRE(std::string(base.what()) == "test message");
}

TEST_CASE("FormException stores error code", "[errors]")
{
	auto ex = FormException(FormErrorCode::FileNotFound, "not found");
	REQUIRE(ex.code() == FormErrorCode::FileNotFound);
	REQUIRE(std::string(ex.what()) == "not found");
}

TEST_CASE("FormException is catchable as std::exception", "[errors]")
{
	try
	{
		throw FormException(FormErrorCode::WindowCreationFailed, "window failed");
	}
	catch (const std::exception& e)
	{
		REQUIRE(std::string(e.what()) == "window failed");
		return;
	}
	FAIL("FormException was not caught as std::exception");
}

// === FormErrorCode enum values ===

TEST_CASE("FormErrorCode has all expected values", "[errors]")
{
	// Verify all enum values exist and are distinct.
	auto codes = std::vector<FormErrorCode>{
		FormErrorCode::ParseError,
		FormErrorCode::InvalidField,
		FormErrorCode::UnknownControlType,
		FormErrorCode::FileNotFound,
		FormErrorCode::FileWriteError,
		FormErrorCode::WindowCreationFailed,
		FormErrorCode::ClassRegistrationFailed,
	};
	// Use a set to verify uniqueness.
	auto unique = std::set<int>{};
	for (auto c : codes)
		unique.insert(static_cast<int>(c));
	REQUIRE(unique.size() == 7);
}

// === TryParseForm ===

TEST_CASE("TryParseForm succeeds on valid JSON", "[errors]")
{
	auto json = std::string(R"({"title":"Test","width":400,"height":300,"controls":[]})");
	auto result = TryParseForm(json);
	REQUIRE(result.has_value());
	REQUIRE(result->title == L"Test");
	REQUIRE(result->width == 400);
}

TEST_CASE("TryParseForm returns ParseError on malformed JSON", "[errors]")
{
	auto result = TryParseForm("{ not valid json }}}");
	REQUIRE(!result.has_value());
	REQUIRE(result.error().code() == FormErrorCode::ParseError);
}

TEST_CASE("TryParseForm returns UnknownControlType for bad type", "[errors]")
{
	auto json = std::string(R"({"title":"T","width":400,"height":300,"controls":[{"type":"FakeWidget","rect":[0,0,100,30]}]})");
	auto result = TryParseForm(json);
	REQUIRE(!result.has_value());
	REQUIRE(result.error().code() == FormErrorCode::UnknownControlType);
}

TEST_CASE("TryParseForm returns InvalidField for missing control type", "[errors]")
{
	auto json = std::string(R"({"title":"T","width":400,"height":300,"controls":[{"text":"No Type","rect":[0,0,100,30]}]})");
	auto result = TryParseForm(json);
	REQUIRE(!result.has_value());
	REQUIRE(result.error().code() == FormErrorCode::InvalidField);
}

TEST_CASE("TryParseForm returns error with descriptive message", "[errors]")
{
	auto result = TryParseForm("not json at all");
	REQUIRE(!result.has_value());
	auto msg = std::string(result.error().what());
	REQUIRE(msg.find("JSON parse error") != std::string::npos);
}

// === ParseForm (throwing) ===

TEST_CASE("ParseForm throws FormException on malformed JSON", "[errors]")
{
	REQUIRE_THROWS_AS(ParseForm("{{bad}}"), FormException);
}

TEST_CASE("ParseForm throws FormException with correct code for unknown type", "[errors]")
{
	auto json = std::string(R"({"title":"T","width":400,"height":300,"controls":[{"type":"Bogus","rect":[0,0,100,30]}]})");
	try
	{
		ParseForm(json);
		FAIL("Expected FormException");
	}
	catch (const FormException& e)
	{
		REQUIRE(e.code() == FormErrorCode::UnknownControlType);
	}
}

TEST_CASE("ParseControlType throws FormException for unknown type", "[errors]")
{
	REQUIRE_THROWS_AS(ParseControlType("NonExistent"), FormException);
	try
	{
		ParseControlType("Nope");
	}
	catch (const FormException& e)
	{
		REQUIRE(e.code() == FormErrorCode::UnknownControlType);
	}
}

// === TryLoadFormFromFile ===

TEST_CASE("TryLoadFormFromFile returns FileNotFound for missing file", "[errors]")
{
	auto result = TryLoadFormFromFile("C:\\nonexistent\\path\\form.json");
	REQUIRE(!result.has_value());
	REQUIRE(result.error().code() == FormErrorCode::FileNotFound);
}

TEST_CASE("LoadFormFromFile throws FormException for missing file", "[errors]")
{
	try
	{
		LoadFormFromFile("C:\\nonexistent\\path\\form.json");
		FAIL("Expected FormException");
	}
	catch (const FormException& e)
	{
		REQUIRE(e.code() == FormErrorCode::FileNotFound);
	}
}

// === TrySaveFormToFile ===

TEST_CASE("TrySaveFormToFile returns FileWriteError for invalid path", "[errors]")
{
	auto form = Form{};
	form.title = L"Test";
	form.width = 400;
	form.height = 300;

	auto result = TrySaveFormToFile(form, "Z:\\nonexistent\\dir\\form.json");
	REQUIRE(!result.has_value());
	REQUIRE(result.error().code() == FormErrorCode::FileWriteError);
}

TEST_CASE("SaveFormToFile throws FormException for invalid path", "[errors]")
{
	auto form = Form{};
	form.title = L"Test";

	REQUIRE_THROWS_AS(SaveFormToFile(form, "Z:\\nonexistent\\dir\\form.json"), FormException);
}

// === TryLoadForm (live window) ===

static struct ErrorTestCommonControlInit {
	ErrorTestCommonControlInit()
	{
		Win32::INITCOMMONCONTROLSEX icc{ sizeof(Win32::INITCOMMONCONTROLSEX), Win32::Icc_StandardClasses };
		Win32::InitCommonControlsEx(&icc);
	}
} g_errorCcInit;

static EventMap g_errorEvents;

TEST_CASE("TryLoadForm succeeds with valid form", "[errors]")
{
	auto form = Form{};
	form.title = L"Test Window";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	auto result = TryLoadForm(form, hInstance, g_errorEvents);
	REQUIRE(result.has_value());
	REQUIRE(*result != nullptr);

	Win32::DestroyWindow(*result);
}

TEST_CASE("LoadForm throws instead of returning nullptr", "[errors]")
{
	// LoadForm should throw FormException on failure rather than returning nullptr.
	// Verify that LoadForm with a valid form succeeds (doesn't throw).
	auto form = Form{};
	form.title = L"OK Window";
	form.width = 300;
	form.height = 200;
	form.visible = false;

	auto hInstance = Win32::GetModuleHandleW(nullptr);
	Win32::HWND hwnd = nullptr;
	REQUIRE_NOTHROW(hwnd = LoadForm(form, hInstance, g_errorEvents));
	REQUIRE(hwnd != nullptr);
	Win32::DestroyWindow(hwnd);
}

// === Backward compatibility ===

TEST_CASE("ParseControlType exception is catchable as std::runtime_error", "[errors]")
{
	// Existing code catching std::runtime_error should still work.
	REQUIRE_THROWS_AS(ParseControlType("UnknownWidget"), std::runtime_error);
}

TEST_CASE("LoadFormFromFile exception is catchable as std::exception", "[errors]")
{
	REQUIRE_THROWS_AS(LoadFormFromFile("C:\\no\\such\\file.json"), std::exception);
}
