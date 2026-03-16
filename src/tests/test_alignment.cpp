#include <catch2/catch_test_macros.hpp>
import std;
import formbuilder;
import designer;

using namespace FormDesigner;
using namespace Designer;

// Helper: create controls with specified rects.
static auto MakeControls(std::vector<Rect> rects) -> std::vector<Control>
{
    auto controls = std::vector<Control>{};
    for (auto& r : rects)
    {
        Control c;
        c.type = ControlType::Button;
        c.rect = r;
        controls.push_back(std::move(c));
    }
    return controls;
}

static auto Ptrs(std::vector<Control>& controls) -> std::vector<Control*>
{
    auto result = std::vector<Control*>{};
    for (auto& c : controls) result.push_back(&c);
    return result;
}

// === Alignment Tests ===

TEST_CASE("AlignLeft aligns all controls to the leftmost x", "[alignment]")
{
    auto controls = MakeControls({{50, 10, 80, 20}, {100, 30, 60, 20}, {30, 50, 70, 20}});
    auto ptrs = Ptrs(controls);
    AlignLeft(ptrs);
    REQUIRE(controls[0].rect.x == 30);
    REQUIRE(controls[1].rect.x == 30);
    REQUIRE(controls[2].rect.x == 30);
}

TEST_CASE("AlignRight aligns all right edges to the rightmost", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 80, 20}, {50, 30, 100, 20}, {20, 50, 60, 20}});
    auto ptrs = Ptrs(controls);
    AlignRight(ptrs);
    // Rightmost edge is 50+100 = 150
    REQUIRE(controls[0].rect.x == 70);  // 150 - 80
    REQUIRE(controls[1].rect.x == 50);  // 150 - 100
    REQUIRE(controls[2].rect.x == 90);  // 150 - 60
}

TEST_CASE("AlignCenterH centers all controls horizontally", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 40, 20}, {80, 30, 60, 20}});
    auto ptrs = Ptrs(controls);
    AlignCenterH(ptrs);
    // Bounding box: x=10 to x=140 (80+60), center = 75
    REQUIRE(controls[0].rect.x == 55);  // 75 - 40/2
    REQUIRE(controls[1].rect.x == 45);  // 75 - 60/2
}

TEST_CASE("AlignTop aligns all controls to the topmost y", "[alignment]")
{
    auto controls = MakeControls({{10, 50, 80, 20}, {30, 20, 60, 30}, {50, 80, 70, 25}});
    auto ptrs = Ptrs(controls);
    AlignTop(ptrs);
    REQUIRE(controls[0].rect.y == 20);
    REQUIRE(controls[1].rect.y == 20);
    REQUIRE(controls[2].rect.y == 20);
}

TEST_CASE("AlignBottom aligns all bottom edges to the bottommost", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 80, 20}, {30, 50, 60, 40}, {50, 20, 70, 10}});
    auto ptrs = Ptrs(controls);
    AlignBottom(ptrs);
    // Bottommost edge is 50+40 = 90
    REQUIRE(controls[0].rect.y == 70);  // 90 - 20
    REQUIRE(controls[1].rect.y == 50);  // 90 - 40
    REQUIRE(controls[2].rect.y == 80);  // 90 - 10
}

TEST_CASE("AlignMiddleV centers all controls vertically", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 40, 20}, {30, 80, 60, 40}});
    auto ptrs = Ptrs(controls);
    AlignMiddleV(ptrs);
    // Bounding box: y=10 to y=120 (80+40), middle = 65
    REQUIRE(controls[0].rect.y == 55);  // 65 - 20/2
    REQUIRE(controls[1].rect.y == 45);  // 65 - 40/2
}

TEST_CASE("Alignment requires at least 2 controls (no-op on 1)", "[alignment]")
{
    auto controls = MakeControls({{50, 50, 80, 20}});
    auto ptrs = Ptrs(controls);
    AlignLeft(ptrs);
    REQUIRE(controls[0].rect.x == 50);
}

// === Distribution Tests ===

TEST_CASE("DistributeHorizontally spaces controls evenly", "[alignment]")
{
    // Three controls of width 20, spanning from x=10 to x=130 (110+20)
    auto controls = MakeControls({{10, 10, 20, 20}, {110, 10, 20, 20}, {50, 10, 20, 20}});
    auto ptrs = Ptrs(controls);
    DistributeHorizontally(ptrs);
    // Total width span: 130 - 10 = 120, total control width: 60, space: 60, 2 gaps = 30 each
    REQUIRE(controls[0].rect.x == 10);   // leftmost stays
    REQUIRE(controls[2].rect.x == 60);   // middle (sorted by x)
    REQUIRE(controls[1].rect.x == 110);  // rightmost stays
}

TEST_CASE("DistributeVertically spaces controls evenly", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 20, 20}, {10, 110, 20, 20}, {10, 50, 20, 20}});
    auto ptrs = Ptrs(controls);
    DistributeVertically(ptrs);
    REQUIRE(controls[0].rect.y == 10);
    REQUIRE(controls[2].rect.y == 60);
    REQUIRE(controls[1].rect.y == 110);
}

TEST_CASE("Distribution requires at least 3 controls (no-op on 2)", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 20, 20}, {100, 10, 20, 20}});
    auto ptrs = Ptrs(controls);
    DistributeHorizontally(ptrs);
    REQUIRE(controls[0].rect.x == 10);
    REQUIRE(controls[1].rect.x == 100);
}

// === Sizing Tests ===

TEST_CASE("MakeSameWidth sets all widths to the maximum", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 50, 20}, {30, 30, 100, 25}, {50, 50, 80, 30}});
    auto ptrs = Ptrs(controls);
    MakeSameWidth(ptrs);
    REQUIRE(controls[0].rect.width == 100);
    REQUIRE(controls[1].rect.width == 100);
    REQUIRE(controls[2].rect.width == 100);
}

TEST_CASE("MakeSameHeight sets all heights to the maximum", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 50, 20}, {30, 30, 60, 40}, {50, 50, 70, 30}});
    auto ptrs = Ptrs(controls);
    MakeSameHeight(ptrs);
    REQUIRE(controls[0].rect.height == 40);
    REQUIRE(controls[1].rect.height == 40);
    REQUIRE(controls[2].rect.height == 40);
}

TEST_CASE("MakeSameSize sets both width and height to maximums", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 100, 20}, {30, 30, 50, 60}});
    auto ptrs = Ptrs(controls);
    MakeSameSize(ptrs);
    REQUIRE(controls[0].rect.width == 100);
    REQUIRE(controls[0].rect.height == 60);
    REQUIRE(controls[1].rect.width == 100);
    REQUIRE(controls[1].rect.height == 60);
}

// === Locked Controls ===

TEST_CASE("CollectUnlocked skips locked controls", "[alignment]")
{
    auto controls = MakeControls({{10, 10, 50, 20}, {50, 10, 60, 20}, {90, 10, 70, 20}});
    controls[1].locked = true;
    auto selection = std::set<int>{0, 1, 2};
    auto ptrs = CollectUnlocked(controls, selection);
    REQUIRE(ptrs.size() == 2);
    REQUIRE(ptrs[0] == &controls[0]);
    REQUIRE(ptrs[1] == &controls[2]);
}

TEST_CASE("AlignLeft with locked control skipped via CollectUnlocked", "[alignment]")
{
    auto controls = MakeControls({{50, 10, 80, 20}, {100, 30, 60, 20}, {30, 50, 70, 20}});
    controls[2].locked = true;
    auto selection = std::set<int>{0, 1, 2};
    auto ptrs = CollectUnlocked(controls, selection);
    AlignLeft(ptrs);
    REQUIRE(controls[0].rect.x == 50);  // aligned to min of unlocked (50)
    REQUIRE(controls[1].rect.x == 50);
    REQUIRE(controls[2].rect.x == 30);  // locked, unchanged
}
