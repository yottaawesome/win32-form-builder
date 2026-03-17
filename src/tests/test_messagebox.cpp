#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;

using namespace FormDesigner;

// === DialogResult ↔ Win32 ID mapping tests ===

TEST_CASE("DialogResult::Ok matches IDOK", "[messagebox]")
{
	REQUIRE(static_cast<int>(DialogResult::Ok) == Win32::Id_Ok);
}

TEST_CASE("DialogResult::Cancel matches IDCANCEL", "[messagebox]")
{
	REQUIRE(static_cast<int>(DialogResult::Cancel) == Win32::Id_Cancel);
}

TEST_CASE("DialogResult::Yes matches IDYES", "[messagebox]")
{
	REQUIRE(static_cast<int>(DialogResult::Yes) == Win32::Id_Yes);
}

TEST_CASE("DialogResult::No matches IDNO", "[messagebox]")
{
	REQUIRE(static_cast<int>(DialogResult::No) == Win32::Id_No);
}

// === Compile/link verification ===
// These verify the helpers are correctly exported and callable.
// We can't invoke them in tests (they'd show a modal dialog), but we
// can take their addresses to confirm linkage.

TEST_CASE("ShowInfo is callable", "[messagebox]")
{
	auto fn = &ShowInfo;
	REQUIRE(fn != nullptr);
}

TEST_CASE("ShowError is callable", "[messagebox]")
{
	auto fn = &ShowError;
	REQUIRE(fn != nullptr);
}

TEST_CASE("ShowWarning is callable", "[messagebox]")
{
	auto fn = &ShowWarning;
	REQUIRE(fn != nullptr);
}

TEST_CASE("AskYesNo is callable", "[messagebox]")
{
	auto fn = &AskYesNo;
	REQUIRE(fn != nullptr);
}

TEST_CASE("AskYesNoCancel is callable", "[messagebox]")
{
	auto fn = &AskYesNoCancel;
	REQUIRE(fn != nullptr);
}

TEST_CASE("AskOkCancel is callable", "[messagebox]")
{
	auto fn = &AskOkCancel;
	REQUIRE(fn != nullptr);
}

// === Win32 MB_ constant consistency ===

TEST_CASE("MB button style constants are distinct", "[messagebox]")
{
	REQUIRE(Win32::Mb_Ok != Win32::Mb_OkCancel);
	REQUIRE(Win32::Mb_OkCancel != Win32::Mb_YesNo);
	REQUIRE(Win32::Mb_YesNo != Win32::Mb_YesNoCancel);
}

TEST_CASE("MB icon constants are distinct", "[messagebox]")
{
	REQUIRE(Win32::Mb_IconInformation != Win32::Mb_IconError);
	REQUIRE(Win32::Mb_IconError != Win32::Mb_IconWarning);
	REQUIRE(Win32::Mb_IconWarning != Win32::Mb_IconQuestion);
}
