#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;
import designer;

using namespace Designer;

// === ScaleDpi ===

TEST_CASE("ScaleDpi returns base value at 96 DPI", "[dpi]")
{
	REQUIRE(Win32::ScaleDpi(10, 96) == 10);
	REQUIRE(Win32::ScaleDpi(20, 96) == 20);
	REQUIRE(Win32::ScaleDpi(140, 96) == 140);
	REQUIRE(Win32::ScaleDpi(0, 96) == 0);
}

TEST_CASE("ScaleDpi scales correctly at 144 DPI (150%)", "[dpi]")
{
	REQUIRE(Win32::ScaleDpi(10, 144) == 15);
	REQUIRE(Win32::ScaleDpi(20, 144) == 30);
	REQUIRE(Win32::ScaleDpi(140, 144) == 210);
	REQUIRE(Win32::ScaleDpi(6, 144) == 9);
}

TEST_CASE("ScaleDpi scales correctly at 192 DPI (200%)", "[dpi]")
{
	REQUIRE(Win32::ScaleDpi(10, 192) == 20);
	REQUIRE(Win32::ScaleDpi(20, 192) == 40);
	REQUIRE(Win32::ScaleDpi(140, 192) == 280);
	REQUIRE(Win32::ScaleDpi(6, 192) == 12);
}

// === DpiInfo ===

TEST_CASE("DpiInfo typed accessors return base values at 96 DPI", "[dpi]")
{
	DpiInfo info{ .dpi = 96 };

	REQUIRE(info.HandleSize() == BASE_HANDLE_SIZE);
	REQUIRE(info.HandleHalf() == BASE_HANDLE_HALF);
	REQUIRE(info.MinControlSize() == BASE_MIN_CONTROL_SIZE);
	REQUIRE(info.SnapThreshold() == BASE_SNAP_THRESHOLD);
	REQUIRE(info.RulerSize() == BASE_RULER_SIZE);
	REQUIRE(info.ToolboxWidth() == BASE_TOOLBOX_WIDTH);
	REQUIRE(info.PropertyWidth() == BASE_PROPERTY_WIDTH);
}

TEST_CASE("DpiInfo typed accessors return 2x values at 192 DPI", "[dpi]")
{
	DpiInfo info{ .dpi = 192 };

	REQUIRE(info.HandleSize() == BASE_HANDLE_SIZE * 2);
	REQUIRE(info.HandleHalf() == BASE_HANDLE_HALF * 2);
	REQUIRE(info.MinControlSize() == BASE_MIN_CONTROL_SIZE * 2);
	REQUIRE(info.SnapThreshold() == BASE_SNAP_THRESHOLD * 2);
	REQUIRE(info.RulerSize() == BASE_RULER_SIZE * 2);
	REQUIRE(info.ToolboxWidth() == BASE_TOOLBOX_WIDTH * 2);
	REQUIRE(info.PropertyWidth() == BASE_PROPERTY_WIDTH * 2);
}

TEST_CASE("DpiInfo Scale method works for arbitrary values", "[dpi]")
{
	DpiInfo info{ .dpi = 120 }; // 125%

	REQUIRE(info.Scale(10) == 13); // MulDiv(10, 120, 96) = 13 (rounds up)
	REQUIRE(info.Scale(96) == 120);
	REQUIRE(info.Scale(0) == 0);
}

TEST_CASE("DpiInfo default DPI is 96", "[dpi]")
{
	DpiInfo info{};
	REQUIRE(info.dpi == 96);
	REQUIRE(info.Scale(100) == 100);
}
