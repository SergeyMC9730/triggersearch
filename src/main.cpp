#include <Geode/Geode.hpp>
#include <alphalaneous.editortab_api/include/EditorTabs.hpp>
#include <map>
#include <vector>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

#include <vector>
#include <map>
#include <assert.h>
#include <cmath>

class TriggerCategory {
private:
    ccColor3B _col = { 255, 255, 255 };
public:
    TriggerCategory() {}
    TriggerCategory(const std::string &cat) {
        setCategory(cat);
    }

    void setCategory(const std::string& cat) {
        // misc ; tint ; fade ; shader ; sound ; area ; enter ; camera ; transition ; scale ; item ; move

        if (cat == "misc") {
            _col = { 148, 242, 238 };
        }
        else if (cat == "tint") {
            _col = { 229, 242, 148 };
        }
        else if (cat == "fade") {
            _col = { 148, 165, 242 };
        }
        else if (cat == "shader") {
            _col = { 237, 154, 154 };
        }
        else if (cat == "sound") {
            _col = { 45, 194, 94 };
        }
        else if (cat == "area") {
            _col = { 194, 184, 45 };
        }
        else if (cat == "enter") {
            _col = { 194, 102, 45 };
        }
        else if (cat == "camera") {
            _col = { 92, 255, 252 };
        }
        else if (cat == "transition") {
            _col = { 255, 239, 92 };
        }
        else if (cat == "scale") {
            _col = { 144, 115, 240 };
        }
        else if (cat == "item") {
            _col = { 95, 98, 227 };
        }
        else if (cat == "move") {
            _col = { 217, 135, 232 };
        }
    }

    ccColor3B getColor() {
        return _col;
    }
};

class TextGradient {
public:
    using Color = ccColor3B;
private:
    struct GradStep {
        int steps;
        Color col;
        Color nextCol;
    };

    std::vector<Color> _gradientParts;
    std::vector<GradStep> _gradientPartsS;
    unsigned int _steps;

    Color _currentColor;
    std::vector<Color> _processedSteps;

    float calculateValueMix(float a, float b, float equality) {
        return a + ((b - a) * equality);
    }

    Color calculateMix(Color a, Color b, float equality) {
        // prevent nan and equality overflow situations
        if (equality >= 1.f) return b;
        if (equality <= 0.f) return a;

        Color newc;

        newc.r = (unsigned char)calculateValueMix((float)a.r, (float)b.r, equality);
        newc.g = (unsigned char)calculateValueMix((float)a.g, (float)b.g, equality);
        newc.b = (unsigned char)calculateValueMix((float)a.b, (float)b.b, equality);

        // log::info("mixed colors {} and {} with {} -> got {}", a, b, equality, newc);

        return newc;
    }

    void renderGradient() {
        assert(_steps != 0);

        _processedSteps.clear();

        if (_gradientParts.size() == 1) {
            for (int i = 0; i < _steps; i++) {
                _processedSteps.push_back(_gradientParts[0]);
            }

            return;
        }

        for (GradStep& step : _gradientPartsS) {
            for (int i = 0; i < step.steps; i++) {
                _processedSteps.push_back(calculateMix(step.col, step.nextCol, (float)i / (float)step.steps));
            }
        }
    }

    void debugSteps() {
        // log::debug("{} keyframes created.", _gradientPartsS.size());
        for (GradStep& s : _gradientPartsS) {
            // log::debug(" - color {} for {} steps. Fades into {}", s.col, s.steps, s.nextCol);
        }
    }

    void setupGradientSteps() {
        GradStep currentStep = {};
        int stepsPerColor = (_steps * 1.5) / _gradientParts.size();
    
        int simulatedSteps = 0;

        int idx = 0;
        for (auto& color : _gradientParts) {
            currentStep.col = color;
            currentStep.steps = stepsPerColor;

            bool last = idx == _gradientParts.size() - 1;
            if (last) {
                currentStep.nextCol = currentStep.col;
            }
            else {
                currentStep.nextCol = _gradientParts[idx + 1];
            }

            _gradientPartsS.push_back(currentStep);

            simulatedSteps += stepsPerColor;

            idx++;
        }

        _gradientPartsS[_gradientPartsS.size() - 1].steps += _steps - simulatedSteps;
    }
public:
    TextGradient(unsigned int steps, std::vector<TextGradient::Color> parts) {
        _gradientParts = parts;
        _steps = steps;

        assert(!parts.empty());
        _currentColor = parts[0];

        setupGradientSteps();
        // debugSteps();

        renderGradient();
    }
    TextGradient() {}

    TextGradient::Color colorAtStep(unsigned int step) {
        assert(_processedSteps.size() != 0);

        return _processedSteps[step];
    }
};

class SObject {
public:
    std::string _name;
    int _objId;
    std::vector<std::string> _texturePath = {};
    std::vector<TriggerCategory> _categories = {};
    bool _extraArg = false;

    CCNode* loadSprite() const {
        CCNode* nd = CCNode::create();
        
        for (std::string p : _texturePath) {
            CCSprite* spr = CCSprite::createWithSpriteFrameName(p.c_str());
            if (!spr) {
                spr = CCSprite::create(p.c_str());
            }

            if (!spr) continue;

            if (p == "edit_eToggleBtn2_001.png") {
                if (_extraArg) {
                    spr->setColor({ 0, 255, 0 });
                }
                else {
                    spr->setColor({ 255, 0, 0 });
                }
            }

            spr->setPosition(spr->getContentSize() / 2.f);

            if (p == "edit_eToggleBtn_001.png") {
                spr->setPositionX(10.f);
            }
            else if (p == "edit_eLevelEndBtn_001.png"_spr) {
                spr->setScale(2.f);
            }

            nd->addChild(spr);
            nd->setContentSize(spr->getContentSize());
        }

        return nd;
    }
};

static int __currentTriggerIdx = 0;
static int __copiedTriggerID = -1;
static const std::map<int, SObject> __triggerObjects = {
    {22,   {"No Transition", 22, {"edit_eeNoneBtn_001.png"}, {{"transition"}}}},
    {23,   {"Bottom Transition", 23, {"edit_eeFBBtn_001.png"}, {{"transition"}}}},
    {24,   {"Top Transition", 24, {"edit_eeFTBtn_001.png"}, {{"transition"}}}},
    {25,   {"Left Transition", 25, {"edit_eeFLBtn_001.png"}, {{"transition"}}}},
    {26,   {"Right Transition", 26, {"edit_eeFRBtn_001.png"}, {{"transition"}}}},
    {27,   {"Size Increase Transition", 27, {"edit_eeSUBtn_001.png"}, {{"transition"}}}},
    {28,   {"Size Decrease Transition", 28, {"edit_eeSDBtn_001.png"}, {{"transition"}}}},
    {29,   {"Legacy BG Tint 1", 29, {"edit_eTintCol01Btn_001.png"}, {{"tint"}}}},
    {30,   {"Legacy Ground Tint 1", 30, {"edit_eTintCol01Btn_001.png"}, {{"tint"}}}},
    {31,   {"Start Pos", 31, {"edit_eStartPosBtn_001.png"}, {{"misc"}}}},
    {32,   {"Ghost Enable", 32, {"edit_eGhostEBtn_001.png"}, {{"misc"}}}},
    {33,   {"Ghost Disable", 33, {"edit_eGhostDBtn_001.png"}, {{"misc"}}}},
    {34,   {"Legacy Level End", 34, {"edit_eLevelEndBtn_001.png"_spr}, {{"misc"}}}},
    {55,   {"Randomized Transition", 55, {"edit_eeFABtn_001.png"}, {{"transition"}}}},
    {56,   {"Far Right Transition", 56, {"edit_eeFALBtn_001.png"}, {{"transition"}}}},
    {57,   {"Far Left Transition", 57, {"edit_eeFARBtn_001.png"}, {{"transition"}}}},
    {58,   {"Mutual Transition", 58, {"edit_eeFRHBtn_001.png"}, {{"transition"}}}},
    {59,   {"Reversed Mutual Transition", 59, {"edit_eeFRHInvBtn_001.png"}, {{"transition"}}}},
    {899,  {"Color Tint", 899, {"edit_eTintCol01Btn_001.png"}, {{"tint"}}}},
    {900,  {"Legacy Tint 10", 900, {"edit_eTintCol01Btn_001.png"}, {{"tint"}}}},
    {901,  {"Move", 901, {"edit_eMoveComBtn_001.png"}, {{"move"}}}},
    {915,  {"Legacy Tint 11", 915, {"edit_eTintCol01Btn_001.png"}, {{"tint"}}}},
    {1006, {"Pulse", 1006, {"edit_ePulseBtn_001.png"}, {{"tint"}}}},
    {1007, {"Alpha", 1007, {"edit_eAlphaBtn_001.png"}, {{"fade"}}}},
    {1049, {"Toggle", 1049, {"edit_eToggleBtn_001.png", "edit_eToggleBtn2_001.png"}, {{"misc"}}}},
    {1268, {"Spawn", 1268, {"edit_eSpawnBtn_001.png"}, {{"misc"}}}},
    {1347, {"Follow", 1347, {"edit_eFollowComBtn_001.png"}, {{"move"}}}},
    {1520, {"Shake", 1520, {"edit_eShakeBtn_001.png"}, {{"misc"}}}},
    {1585, {"Animate", 1585, {"edit_eAnimateBtn_001.png"}, {{"misc"}}}},
    {1595, {"Touch", 1595, {"edit_eTouchBtn_001.png"}, {{"misc"}}}},
    {1611, {"Count", 1611, {"edit_eStopMoverBtn_001.png"}, {{"item"}}}},
    {1612, {"Hide Player", 1612, {"edit_ePHideBtn_001.png"}, {{"misc"}}}},
    {1613, {"Show Player", 1613, {"edit_ePShowBtn_001.png"}, {{"misc"}}}},
    {1616, {"Stop", 1616, {"edit_eCountBtn_001.png"}, {{"misc"}}}},
    {1811, {"Instant Count", 1811, {"edit_eInstantCountBtn_001.png"}, {{"item"}}}},
    {1812, {"On Death", 1812, {"edit_eOnDeathBtn_001.png"}, {{"misc"}}}},
    {1814, {"Follow Player Y", 1814, {"edit_eFollowPComBtn_001.png"}, {{"move"}}}},
    {1815, {"Collision", 1815, {"edit_eCollisionBtn_001.png"}, {{"misc"}}}},
    {1817, {"Pickup", 1817, {"edit_ePickupBtn_001.png"}, {{"item"}}}},
    {1818, {"BG Effect (on)", 1818, {"edit_eBGEOn_001.png"}, {{"misc"}}}},
    {1819, {"BG Effect (off)", 1819, {"edit_eBGEOff_001.png"}, {{"misc"}}}},
    {1912, {"Random", 1912, {"edit_eRandomBtn_001.png"}, {{"misc"}}}},
    {1913, {"Camera Zoom", 1913, {"edit_eZoomBtn_001.png"}, {{"camera"}}}},
    {1914, {"Static Camera", 1914, {"edit_eStaticBtn_001.png"}, {{"camera"}}}},
    {1915, {"Enter Effect", 1915, {"edit_eeNone2Btn_001.png"}, {{"misc"}}}},
    {1916, {"Camera Offset", 1916, {"edit_eOffsetBtn_001.png"}, {{"camera"}}}},
    {1917, {"Reverse", 1917, {"edit_eReverseBtn_001.png"}, {{"misc"}}}},
    {1931, {"Level End 1", 1931, {"edit_eEndBtn_001.png"}, {{"misc"}}}},
    {1932, {"Player Control", 1932, {"edit_eReleaseJumpBtn_001.png"}, {{"misc"}}}},
    {1934, {"Song Change", 1934, {"edit_eSongBtn_001.png"}, {{"sound"}}}},
    {1935, {"Timewarp", 1935, {"edit_eTimeWarpBtn_001.png"}, {{"misc"}}}},
    {2015, {"Camera Rotate", 2015, {"edit_eCamRotBtn_001.png"}, {{"camera"}}}},
    {2016, {"Camera Guide", 2016, {"edit_eCamGuideBtn_001.png"}, {{"camera"}}}},
    {2062, {"Camera Edge", 2062, {"edit_eEdgeBtn_001.png"}, {{"camera"}}}},
    {2066, {"Gravity", 2066, {"edit_eGravity_001.png"}, {{"misc"}}}},
    {2067, {"Scale", 2067, {"edit_eScaleComBtn_001.png"}, {{"misc"}}}},
    {2068, {"Advanced Random", 2068, {"edit_eAdvRandomBtn_001.png"}, {{"misc"}}}},
    {2899, {"Options", 2899, {"edit_eOptionsBtn_001.png"}, {{"misc"}}}},
    {2900, {"Rotate Gameplay", 2900, {"edit_eGameRotBtn_001.png"}, {{"misc"}}}},
    {2901, {"Gameplay Offset", 2901, {"edit_eGPOffsetBtn_001.png"}, {{"camera"}}}},
    {2903, {"Gradient", 2903, {"edit_eGradientBtn_001.png"}, {{"misc"}}}},
    {2903, {"Setup Shader", 2903, {"edit_eShaderBtn_001.png"}, {{"shader"}, {"misc"}}}},
    {2905, {"Shock Wave Shader", 2905, {"edit_eSh_ShockWaveBtn_001.png"}, {{"shader"}}}},
    {2907, {"Shock Line Shader", 2907, {"edit_eSh_ShockLineBtn_001.png"}, {{"shader"}}}},
    {2909, {"Glitch Shader", 2909, {"edit_eSh_GlitchBtn_001.png"}, {{"shader"}}}},
    {2910, {"Chromatic Shader", 2910, {"edit_eSh_ChromaticBtn_001.png"}, {{"shader"}}}},
    {2911, {"Chromatic Glitch Shader", 2911, {"edit_eSh_ChromaticGlitchBtn_001.png"}, {{"shader"}}}},
    {2912, {"Pixelate Shader", 2912, {"edit_eSh_PixelateBtn_001.png"}, {{"shader"}}}},
    {2913, {"Lens Cirlce Shader", 2913, {"edit_eSh_LensCircleBtn_001.png"}, {{"shader"}}}},
    {2914, {"Radial Blur Shader", 2914, {"edit_eSh_RadialBlurBtn_001.png"}, {{"shader"}}}},
    {2915, {"Motion Blur Shader", 2915, {"edit_eSh_MotionBlurBtn_001.png"}, {{"shader"}}}},
    {2916, {"Bulge Shader", 2916, {"edit_eSh_BulgeBtn_001.png"}, {{"shader"}}}},
    {2917, {"Pinch Shader", 2917, {"edit_eSh_PinchBtn_001.png"}, {{"shader"}}}},
    {2919, {"Grayscale Shader", 2919, {"edit_eSh_GrayscaleBtn_001.png"}, {{"shader"}}}},
    {2920, {"Sepia Shader", 2920, {"edit_eSh_SepiaBtn_001.png"}, {{"shader"}}}},
    {2921, {"Invert Color Shader", 2921, {"edit_eSh_InvertColorBtn_001.png"}, {{"shader"}}}},
    {2922, {"Hue Shader", 2922, {"edit_eSh_HueBtn_001.png"}, {{"shader"}}}},
    {2923, {"Edit Color Shader", 2923, {"edit_eSh_EditColorBtn_001.png"}, {{"shader"}, {"tint"}}}},
    {2924, {"Split Screen Shader", 2924, {"edit_eSh_SplitScreenBtn_001.png"}, {{"shader"}}}},
    {2925, {"Camera Mode", 2925, {"edit_eCamModeBtn_001.png"}, {{"camera"}}}},
    {2999, {"Setup Middleground", 2999, {"edit_eSetupMGBtn_001.png"}, {{"misc"}}}},
    {3006, {"Area Move", 3006, {"edit_eAreaMoveBtn_001.png"}, {{"area"}, {"move"}}}},
    {3007, {"Area Rotate", 3007, {"edit_eAreaRotateBtn_001.png"}, {{"area"}}}},
    {3008, {"Area Scale", 3008, {"edit_eAreaScaleBtn_001.png"}, {{"area"}}}},
    {3009, {"Area Fade", 3009, {"edit_eAreaFadeBtn_001.png"}, {{"area"}, {"fade"}}}},
    {3010, {"Area Tint", 3010, {"edit_eAreaTintBtn_001.png"}, {{"area"}, {"tint"}}}},
    {3011, {"Edit Area Move", 3011, {"edit_eEAreaMoveBtn_001.png"}, {{"area"}, {"move"}}}},
    {3012, {"Edit Area Rotate", 3012, {"edit_eEAreaRotateBtn_001.png"}, {{"area"}}}},
    {3013, {"Edit Area Scale", 3013, {"edit_eEAreaScaleBtn_001.png"}, {{ "area" }, {"scale"}}}},
    {3014, {"Edit Area Fade", 3014, {"edit_eEAreaFadeBtn_001.png"}, {{"area"}, {"fade"}}}},
    {3015, {"Edit Area Tint", 3015, {"edit_eEAreaTintBtn_001.png"}, {{"tint"}, {"tint"}}}},
    {3016, {"Advanced Follow", 3016, {"edit_eAdvFollowBtn_001.png"}, {{"move"}}}},
    {3017, {"Enter Move", 3017, {"edit_eEnterMoveBtn_001.png"}, {{"move"}}}},
    {3018, {"Enter Rotate", 3018, {"edit_eEnterRotateBtn_001.png"}, {{"enter"}}}},
    {3019, {"Enter Scale", 3019, {"edit_eEnterScaleBtn_001.png"}, {{"enter"}}}},
    {3020, {"Enter Fade", 3020, {"edit_eEnterFadeBtn_001.png"}, {{"enter"}}}},
    {3021, {"Enter Tint", 3021, {"edit_eEnterTintBtn_001.png"}, {{"tint"}}}},
    {3022, {"Teleport", 3022, {"edit_eTeleportBtn_001.png"}, {{"misc"}}}},
    {3023, {"Enter Stop", 3023, {"edit_eEnterStopBtn_001.png"}, {{"misc"}}}},
    {3024, {"Area Stop", 3024, {"edit_eAreaStopBtn_001.png"}, {{"misc"}}}},
    {3029, {"Change Background", 3029, {"edit_eChangeBG_001.png"}, {{"misc"}}}},
    {3030, {"Change Ground", 3030, {"edit_eChangeG_001.png"}, {{"misc"}}}},
    {3031, {"Change Middleground", 3031, {"edit_eChangeMG_001.png"}, {{"misc"}}}},
    {3033, {"Keyframe Trigger", 3033, {"edit_eKeyframeBtn_001.png"}, {{"misc"}}}},
    {3600, {"Level End 2", 3600, {"edit_eEndBtn_001.png"}, {{"misc"}}}},
    {3602, {"SFX", 3602, {"edit_eSFXBtn_001.png"}, {{"sound"}}}},
    {3603, {"Edit SFX", 3603, {"edit_eEditSFXBtn_001.png"}, {{"sound"}}}},
    {3604, {"Event", 3604, {"edit_eEventLinkBtn_001.png"}, {{"misc"}}}},
    {3605, {"Edit Song", 3605, {"edit_eEditSongBtn_001.png"}, {{"sound"}}}},
    {3606, {"Background Speed", 3606, {"edit_eBGSpeedBtn_001.png"}, {{"misc"}}}},
    {3607, {"Sequence", 3607, {"edit_eSequenceBtn_001.png"}, {{"misc"}}}},
    {3608, {"Spawn Particle", 3608, {"edit_eSpawnParticleBtn_001.png"}, {{"misc"}}}},
    {3609, {"Instant Collision", 3609, {"edit_eInstantCollisionBtn_001.png"}}},
    {3612, {"Middleground Speed", 3612, {"edit_eMGSpeedBtn_001.png"}, {{"misc"}}}},
    {3613, {"UI Settings", 3613, {"edit_eUISettingsBtn_001.png"}, {{"misc"}}}},
    {3614, {"Time", 3614, {"edit_eTimeBtn_001.png"}, {{"misc"}}}},
    {3615, {"Time Event", 3615, {"edit_eTimeEventBtn_001.png"}, {{"misc"}}}},
    {3617, {"Time Control", 3617, {"edit_eTimeControlBtn_001.png"}, {{"misc"}}}},
    {3618, {"Reset", 3618, {"edit_eResetBtn_001.png"}, {{"item"}}}},
    {3619, {"Item Edit", 3619, {"edit_eItemEditBtn_001.png"}, {{"item"}}}},
    {3620, {"Item Compare", 3620, {"edit_eItemCompBtn_001.png"}, {{"item"}}}},
    {3641, {"Persistent Item", 3641, {"edit_eItemPersBtn_001.png"}, {{"item"}}}},
    {3642, {"BPM Guide", 3642, {"edit_eBPMBtn_001.png"}, {{"sound"}, {"misc"}}}},
    {3660, {"Edit Advanced Follow", 3660, {"edit_eEditAdvFollowBtn_001.png"}, {{"move"}, {"misc"}}}},
    {3661, {"Re-Target Advanced Follow", 3661, {"edit_eReAdvFollowBtn_001.png"}, {{"move"}, {"misc"}}}},
    {3662, {"Visibility Link", 3662, {"edit_eLinkVisibleBtn_001.png"}, {{"misc"}}}}
};

#include <memory>

class SObjectCell : public CCNode {
private:
    CCScale9Sprite* _bg;
    cocos2d::CCNode* _infoContainer;
    cocos2d::CCNode* _titleContainer;
    cocos2d::CCLabelBMFont* _titleLabel;
    cocos2d::CCMenu* _buttonMenu;
    CCNode* _objSpr;

    struct SObject _baseObject;

    TextGradient _gradient;

    ButtonSprite* _copySprite;
    CCMenuItemSpriteExtra* _copyButton;

    bool _selectedLocal = false;
public:
    static SObjectCell* create(const struct SObject &obj) {
        SObjectCell* pRet = new SObjectCell(); if (pRet && pRet->init(obj)) {
            pRet->autorelease(); return pRet;
        }
        else {
            delete pRet; pRet = 0; return 0;
        }
    }

    void setColorToBS(ButtonSprite *spr, ccColor3B col) {
        spr->m_label->setColor(col);
        spr->m_BGSprite->setColor(col);
    }

    void update(float delta) {
        EditorUI* ui = EditorUI::get();
        
        if (ui->m_selectedObjectIndex == _baseObject._objId) {
            setColorToBS(_copySprite, { 128, 128, 128 });
            // _copyButton->setEnabled(false);
        }
        else {
            setColorToBS(_copySprite, { 255, 255, 255 });
            // _copyButton->setEnabled(true);
        }
    }

    void onCopy(CCObject* sender) {

        EditorUI* ui = EditorUI::get();

        if (ui->m_selectedObjectIndex == _baseObject._objId) {
            ui->m_selectedObjectIndex = 0;
            _selectedLocal = false;

            return;
        }
        
        ui->m_selectedObjectIndex = _baseObject._objId;
        ui->updateCreateMenu(false);

        auto notification = Notification::create("Object has been selected.", NotificationIcon::Success, 2.f);
        notification->show();

        _selectedLocal = true;
    }

    bool init(const struct SObject& obj) {
        if (!CCNode::init()) return false;

        _baseObject = obj;

        _bg = CCScale9Sprite::create("square02_small.png");
        _bg->setID("bg");
        _bg->setOpacity(80);
        _bg->ignoreAnchorPointForPosition(false);
        _bg->setAnchorPoint({ .5f, .5f });
        _bg->setScale(.7f);
        this->addChild(_bg);

        _infoContainer = CCNode::create();
        _infoContainer->setID("info-container");

        _titleContainer = CCNode::create();

        std::string id_str = obj._name;

        _titleLabel = CCLabelBMFont::create(id_str.c_str(), "bigFont.fnt", CCTextAlignment::kCCTextAlignmentLeft);
        _titleLabel->setID("title-label");
        _titleLabel->setScale(0.5f);
        _titleLabel->setPositionX(40.f);
        _titleLabel->setAnchorPoint({ 0.f, 0.5f });

        std::vector<CCSprite*> _chars = {};
        CCArray* textChildren = _titleLabel->getChildren();

        for (int i = 0; i < textChildren->count(); i++) {
            if (CCSprite* chSpr = typeinfo_cast<CCSprite*>(textChildren->objectAtIndex(i))) {
                _chars.push_back(chSpr);
            }
        }

        std::vector<ccColor3B> textColors;

        if (_baseObject._categories.empty()) {
            textColors = { ccWHITE };
        }
        else {
            for (TriggerCategory& cat : _baseObject._categories) {
                textColors.push_back(cat.getColor());
            }
        }

        _gradient = { (unsigned int)_chars.size(), textColors};

        for (int i = 0; i < _chars.size(); i++) {
            CCSprite* spr = _chars[i];
            spr->setColor(_gradient.colorAtStep(i));
        }

        float req_size = 240.f;

        if (_titleLabel->getContentWidth() > req_size) {
            float sc = _titleLabel->getContentWidth() / req_size;
            _titleLabel->setScale(_titleLabel->getScale() / sc);
        }

        _infoContainer->addChild(_titleLabel);

        ButtonSprite* joinSprite = ButtonSprite::create("Copy", 0.7f);

        _buttonMenu = CCMenu::create();

        CCMenuItemSpriteExtra* joinBtn = CCMenuItemSpriteExtra::create(joinSprite, this, menu_selector(SObjectCell::onCopy));
        joinBtn->setPositionX(18.f);

        _buttonMenu->addChild(joinBtn);
        _buttonMenu->setPositionX(36.f);

        this->addChild(_infoContainer);
        this->addChild(_buttonMenu);
        this->setID("object-list-item");

        _copySprite = joinSprite;
        _copyButton = joinBtn;

        scheduleUpdate();

        return true;
    }

    void setupTriggerSprite(float delta) {
        _objSpr = _baseObject.loadSprite();

        _infoContainer->addChild(_objSpr);

        _objSpr->setPositionX(getContentWidth() - (_objSpr->getContentWidth() / 2.f) - 10.f - 80.f);
        _objSpr->setPositionY(-_objSpr->getContentHeight() / 2.f);
    }

    void updateSize(float width) {
        this->setContentSize({ width, 50.f });

        _bg->setContentSize((m_obContentSize - ccp(6, 0)) / _bg->getScale());
        _bg->setPosition(m_obContentSize / 2);

        CCSize titleSpace{
            m_obContentSize.width / 2 - m_obContentSize.height,
            0
        };

        _infoContainer->setPosition(m_obContentSize.height + 10, m_obContentSize.height / 2);
        _infoContainer->setContentSize(ccp(titleSpace.width, titleSpace.height) / _infoContainer->getScale());
        // _infoContainer->updateLayout();

        _titleContainer->setContentWidth(titleSpace.width / _infoContainer->getScale());
        // _titleContainer->updateLayout();

        _infoContainer->setPosition(m_obContentSize.height + 10, m_obContentSize.height / 2);
        _infoContainer->setContentSize(ccp(titleSpace.width, titleSpace.height) / _infoContainer->getScale());
        // s_infoContainer->updateLayout();

        _buttonMenu->setPosition(m_obContentSize.height + 10, m_obContentSize.height / 2);
        _buttonMenu->setContentSize(ccp(titleSpace.width, titleSpace.height) / _buttonMenu->getScale());
        _buttonMenu->setPositionX(36.f);

        this->updateLayout();

        scheduleOnce(schedule_selector(SObjectCell::setupTriggerSprite), (float)__currentTriggerIdx * 0.1);
    }

    void setBgOpacity(unsigned char op) {
        _bg->setOpacity(op);
    }
};

class SObjectList : public CCNode {
private:
    ScrollLayer* _list;
    cocos2d::CCLayerColor* _bg;
public:
    CREATE_FUNC(SObjectList);

    bool init() {
        if (!CCNode::init()) return false;

        this->setContentSize({ 280, 130 });
        this->setAnchorPoint({ .5f, .5f });
        this->setID("object-list"_spr);

        _bg = CCLayerColor::create({ 42, 90, 168 }, this->getContentWidth(), this->getContentHeight());
        _bg->setOpacity(255);
        _bg->setVisible(false);

        addChild(_bg);

        _list = ScrollLayer::create(getContentSize());
        _list->m_contentLayer->setLayout(
            ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(getContentSize().height)
            ->setGap(2.5f)
        );
        this->addChildAtPosition(_list, Anchor::Bottom, ccp(-_list->getScaledContentWidth() / 2, 0));

        return true;
    }
    void updateSize() {
        for (auto& node : CCArrayExt<CCNode*>(_list->m_contentLayer->getChildren())) {
            if (auto item = typeinfo_cast<SObjectCell*>(node)) {
                // item->updateSize(_list->getContentWidth());
            }
        }

        // Store old relative scroll position (ensuring no divide by zero happens)
        auto oldPositionArea = _list->m_contentLayer->getContentHeight() - _list->getContentHeight();
        auto oldPosition = oldPositionArea > 0.f ?
            _list->m_contentLayer->getPositionY() / oldPositionArea :
            -1.f;

        // Auto-grow the size of the list content
        _list->m_contentLayer->updateLayout();

        // Preserve relative scroll position
        _list->m_contentLayer->setPositionY((
            _list->m_contentLayer->getContentHeight() - _list->getContentHeight()
        ) * oldPosition);
    }

    void addCell(SObjectCell* cell) {
        if (_list == nullptr) {
            log::info("SObjectList: error: _list is nullptr");

            return;
        }

        cell->updateSize(_list->getContentWidth());

        _list->m_contentLayer->addChild(cell);

        if ((_list->m_contentLayer->getChildrenCount() % 2) == 1) {
            cell->setBgOpacity(115);
        }

        updateSize();
    }
    void removeCells() {
        _list->m_contentLayer->removeAllChildrenWithCleanup(true);
    }
};

class $modify(MyEditorUI, EditorUI) {

    struct Fields {
        CCMenu* _mainMenu;
        std::string _currentTrigName;
    };

    bool init(LevelEditorLayer* editorLayer) {

        if (!EditorUI::init(editorLayer)) return false;

        auto director = CCDirector::get();
        auto winsize = director->getWinSize();

        int padding = 10;

        m_fields->_mainMenu = CCMenu::create();
        m_fields->_mainMenu->setPosition({ 0, 0 });
        m_fields->_mainMenu->setContentHeight(90.f);
        m_fields->_mainMenu->setContentWidth(winsize.width - 85 - 96 - (padding * 2));
        m_fields->_mainMenu->setPositionX(85.f + padding);

        CCNode* inputNode = CCNode::create();
        inputNode->setID("trigger-input-node"_spr);

        TextInput* in = TextInput::create(100.f, "Enter trigger name...", "chatFont.fnt");
        in->setPositionX(in->getContentWidth() / 2);
        in->setPositionY(m_fields->_mainMenu->getContentHeight() / 2.f);
        in->setID("trigger-input"_spr);

        inputNode->addChild(in);
        inputNode->setContentSize(in->getContentSize());

        RowLayout* layout = RowLayout::create();
        layout->setAutoGrowAxis(false);
        layout->setAutoScale(false);
        layout->setGap(padding);

        m_fields->_mainMenu->setLayout(layout);

        SObjectList* searchNode = SObjectList::create();

        searchNode->setScale(0.7f);

        __currentTriggerIdx = 0;

        in->setCallback([this, searchNode, in](auto output) {
            m_fields->_currentTrigName = output;

            searchNode->removeCells();

            __currentTriggerIdx = 0;

            for (auto& [k, v] : __triggerObjects) {
                if (utils::string::toLower(v._name).find(output) != std::string::npos) {
                    SObjectCell* cell = SObjectCell::create(v);

                    searchNode->addCell(cell);

                    __currentTriggerIdx++;

                    if (false && __currentTriggerIdx == 1) break;
                }
            }

            std::string triggerStr = "triggers";
            if (__currentTriggerIdx == 1) {
                triggerStr = "trigger";
            }

            in->setLabel(fmt::format("{} {} found", __currentTriggerIdx, triggerStr));
        });

        in->setString(m_fields->_currentTrigName, true);

        m_fields->_mainMenu->addChild(inputNode);
        m_fields->_mainMenu->addChild(searchNode);

        m_fields->_mainMenu->updateLayout();

        m_fields->_mainMenu->setPositionX(winsize.width / 2.f);
        m_fields->_mainMenu->setPositionY(m_fields->_mainMenu->getContentHeight() / 2.f);

        in->setPositionY(in->getContentHeight() / 2.f);

        EditorTabs::get()->addTab(this, TabType::BUILD, "trigger-search-tab"_spr, create_tab_callback(MyEditorUI::loadSearchTab));

        return true;
    }

    CCNode* loadSearchTab(EditorUI* ui, CCMenuItemToggler* toggler) {

        CCLabelBMFont* textLabel = CCLabelBMFont::create("S", "bigFont.fnt");
        textLabel->setScale(0.5f);

        EditorTabUtils::setTabIcon(toggler, textLabel);
        textLabel->setPositionY(textLabel->getPositionY() + 1);

        return m_fields->_mainMenu;
    }
};
