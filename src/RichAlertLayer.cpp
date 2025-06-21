#include "RichAlertLayer.hpp"

RichAlertLayer* RichAlertLayer::create(std::string const& title, std::string const& richText, std::string const& btn1, std::string const& btn2,
    float width, bool scroll, float height, float textScale) {
    auto ret = new RichAlertLayer();
    if (ret && ret->init(title, richText, btn1, btn2, width, scroll, height, textScale)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void RichAlertLayer::show() {
    CCDirector::sharedDirector()->getRunningScene()->addChild(this);
}

bool RichAlertLayer::init(std::string const& p1, std::string const& p2, std::string const& p3, std::string const& p4,
    float p5, bool p6, float p7, float p8) {

    auto parsed = parseRichText(p2);

    if (!FLAlertLayer::init(
        nullptr,
        p1.c_str(),
        parsed.text.c_str(),
        p3.c_str(),
        p4.empty() ? nullptr : p4.c_str(),
        p5, p6, p7, p8))
        return false;

    auto textArea = m_mainLayer->getChildByID("content-text-area");
    if (!textArea) return true;

    auto mbf = textArea->getChildByType<MultilineBitmapFont>(0);
    if (!mbf) return true;

    applyFontStyleTags(mbf, parsed.boldTags, parsed.italicTags);
    applyLinkTags(mbf, parsed.links);
    applyColorTags(mbf, parsed.colors);
    applyUnderlineTags(mbf, parsed.underlines);
    applyStrikeTags(mbf, parsed.strikeTags);

    return true;
}

struct ColorTag {
    size_t start;
    size_t end;
    cocos2d::ccColor3B color;
};

struct UnderlineTag {
    size_t start;
    size_t end;
};

struct ItalicTag {
    size_t start;
    size_t end;
};

struct ParsedText {
    std::string text;
    std::vector<ColorTag> colorTags;
    std::vector<UnderlineTag> underlineTags;
    std::vector<ItalicTag> italicTags;
};

struct StrikeTag {
    size_t start;
    size_t end;
};

RichAlertLayer::ParsedText RichAlertLayer::parseRichText(std::string const& raw) {
    ParsedText result;
    size_t pos = 0;

    std::vector<std::pair<size_t, ccColor3B>> colorStack;
    std::vector<size_t> underlineStack;
    std::vector<size_t> boldStack;
    std::vector<size_t> italicStack;
    std::vector<size_t> strikeStack;
    std::vector<std::pair<size_t, std::string>> linkStack;

    while (pos < raw.size()) {
        if (raw[pos] == '<') {
            size_t tagEnd = raw.find('>', pos);
            if (tagEnd == std::string::npos) break;

            std::string tag = raw.substr(pos + 1, tagEnd - pos - 1);

            if (tag.starts_with("col=")) {
                std::string hex = tag.substr(4);
                if (!hex.empty() && hex[0] == '#') hex.erase(0, 1);

                if (hex.size() == 6) {
                    auto hexToByte = [](const std::string& h) -> GLubyte {
                        return static_cast<GLubyte>(std::strtoul(h.c_str(), nullptr, 16));
                        };
                    ccColor3B col = {
                        hexToByte(hex.substr(0, 2)),
                        hexToByte(hex.substr(2, 2)),
                        hexToByte(hex.substr(4, 2))
                    };
                    colorStack.emplace_back(result.text.length(), col);
                }

                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/col") {
                if (!colorStack.empty()) {
                    auto [start, col] = colorStack.back();
                    colorStack.pop_back();
                    result.colors.push_back(ColorTag{ start, result.text.length(), col });
                }
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "u") {
                underlineStack.push_back(result.text.length());
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/u") {
                if (!underlineStack.empty()) {
                    size_t start = underlineStack.back();
                    underlineStack.pop_back();
                    result.underlines.push_back(UnderlineTag{ start, result.text.length() });
                }
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "b") {
                boldStack.push_back(result.text.length());
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/b") {
                if (!boldStack.empty()) {
                    size_t start = boldStack.back();
                    boldStack.pop_back();
                    result.boldTags.push_back(BoldTag{ start, result.text.length() });
                }
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "i") {
                italicStack.push_back(result.text.length());
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/i") {
                if (!italicStack.empty()) {
                    size_t start = italicStack.back();
                    italicStack.pop_back();
                    result.italicTags.push_back(ItalicTag{ start, result.text.length() });
                }
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "s") {
                strikeStack.push_back(result.text.length());
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/s") {
                if (!strikeStack.empty()) {
                    size_t start = strikeStack.back();
                    strikeStack.pop_back();
                    result.strikeTags.push_back(StrikeTag{ start, result.text.length() });
                }
                pos = tagEnd + 1;
                continue;
            }

            if (tag.starts_with("link=")) {
                std::string url = tag.substr(5);
                linkStack.emplace_back(result.text.length(), url);
                pos = tagEnd + 1;
                continue;
            }

            if (tag == "/link") {
                if (!linkStack.empty()) {
                    auto [start, url] = linkStack.back();
                    linkStack.pop_back();
                    result.links.push_back(LinkTag{ start, result.text.length(), url });
                }
                pos = tagEnd + 1;
                continue;
            }

            result.text += raw.substr(pos, tagEnd - pos + 1);
            pos = tagEnd + 1;
        }
        else {
            result.text += raw[pos];
            ++pos;
        }
    }

    return result;
}




void RichAlertLayer::applyColorTags(MultilineBitmapFont* mbf, std::vector<ColorTag> const& tags) {
    for (const auto& tag : tags) {
        size_t globalIndex = 0;

        for (auto label : CCArrayExt<CCLabelBMFont*>(mbf->getChildren())) {
            if (!label) continue;

            auto glyphs = CCArrayExt<CCFontSprite*>(label->getChildren());
            for (auto glyph : glyphs) {
                if (globalIndex >= tag.start && globalIndex < tag.end) {
                    if (glyph) glyph->setColor(tag.color);
                }
                globalIndex++;
                if (globalIndex >= tag.end) break;
            }
            if (globalIndex >= tag.end) break;
        }
    }
}

void RichAlertLayer::applyUnderlineTags(MultilineBitmapFont* mbf, std::vector<UnderlineTag> const& tags) {
    if (auto existing = mbf->getChildByID("underlineLayer")) {
        existing->removeFromParentAndCleanup(true);
    }
    auto underlineLayer = CCNode::create();
    underlineLayer->setID("underlineLayer");
    mbf->addChild(underlineLayer);

    for (const auto& tag : tags) {
        size_t globalIndex = 0;
        bool foundFirst = false;
        CCPoint leftWorld, rightWorld;
        float glyphHeight = 0;
        CCLabelBMFont* tagLabel = nullptr;

        ccColor4F underlineColor = { 1, 1, 1, 1 };

        for (auto label : CCArrayExt<CCLabelBMFont*>(mbf->getChildren())) {
            if (!label) continue;

            auto glyphs = CCArrayExt<CCFontSprite*>(label->getChildren());
            for (auto glyph : glyphs) {
                if (globalIndex >= tag.start && globalIndex < tag.end) {
                    if (!glyph) {
                        globalIndex++;
                        continue;
                    }

                    auto pos = glyph->getPosition();
                    auto size = glyph->getContentSize();
                    auto worldPos = glyph->getParent()->convertToWorldSpace(pos);

                    float left = worldPos.x - size.width / 2;
                    float right = worldPos.x + size.width / 2;

                    if (!foundFirst) {
                        leftWorld = CCPoint(left, worldPos.y);
                        rightWorld = CCPoint(right, worldPos.y);
                        glyphHeight = size.height;
                        tagLabel = label;

                        auto col = glyph->getColor();
                        underlineColor = {
                            col.r / 255.f,
                            col.g / 255.f,
                            col.b / 255.f,
                            1.f
                        };

                        foundFirst = true;
                    }

                    else {
                        rightWorld.x = right;
                    }
                }
                globalIndex++;
                if (globalIndex >= tag.end) break;
            }
            if (globalIndex >= tag.end) break;
        }

        if (foundFirst && tagLabel) {
            auto underline = CCDrawNode::create();

            auto leftLocal = mbf->convertToNodeSpace(leftWorld);
            auto rightLocal = mbf->convertToNodeSpace(rightWorld);

            float underlineY = tagLabel->getPositionY();

            CCPoint verts[4] = {
                { leftLocal.x - 1.f, underlineY },
                { rightLocal.x + 1.f, underlineY },
                { rightLocal.x + 1.f, underlineY - 0.25f },
                { leftLocal.x - 1.f, underlineY - 0.25f },
            };

            underline->drawPolygon(verts, 4, underlineColor, 0, underlineColor);
            underlineLayer->addChild(underline);
        }
    }
}

template<typename T>
std::vector<T> adjustTagOffsets(std::vector<T> const& tags, size_t offset, size_t lineLength) {
    std::vector<T> adjusted;
    for (auto const& tag : tags) {
        if (tag.end <= offset) continue;
        if (tag.start >= offset + lineLength) continue;
        adjusted.push_back(T{
            std::max(tag.start, offset) - offset,
            std::min(tag.end, offset + lineLength) - offset
            });
    }
    return adjusted;
}


void RichAlertLayer::applyFontStyleTags(
    MultilineBitmapFont* mbf,
    std::vector<BoldTag> const& boldTags,
    std::vector<ItalicTag> const& italicTags
) {
    struct LabelChunk {
        CCLabelBMFont* label;
        std::string text;
        float xOffset;
        float yPos;
    };

    std::vector<LabelChunk> chunks;
    for (auto node : CCArrayExt<CCNode*>(mbf->getChildren())) {
        if (auto* label = static_cast<CCLabelBMFont*>(node)) {
            chunks.push_back({ label, label->getString(), label->getPositionX(), label->getPositionY() });
        }
    }

    const float epsilon = 1.0f;
    std::map<float, std::vector<LabelChunk>> linesMap;

    for (auto& chunk : chunks) {
        bool inserted = false;
        for (auto& [lineY, lineChunks] : linesMap) {
            if (fabs(lineY - chunk.yPos) < epsilon) {
                lineChunks.push_back(chunk);
                inserted = true;
                break;
            }
        }
        if (!inserted) linesMap[chunk.yPos] = { chunk };
    }

    std::vector<std::pair<float, std::vector<LabelChunk>>> lines(linesMap.begin(), linesMap.end());
    std::sort(lines.begin(), lines.end(), [](auto& a, auto& b) { return a.first > b.first; });

    for (auto& chunk : chunks)
        chunk.label->removeFromParentAndCleanup(true);

    float globalOffset = 0;
    for (auto& [lineY, lineChunks] : lines) {
        std::sort(lineChunks.begin(), lineChunks.end(), [](auto& a, auto& b) { return a.xOffset < b.xOffset; });

        std::string lineText;
        for (auto& chunk : lineChunks) lineText += chunk.text;
        
        auto styleMap = buildStyleMap(
            lineText.size(),
            adjustTagOffsets(boldTags, globalOffset, lineText.size()),
            adjustTagOffsets(italicTags, globalOffset, lineText.size())
        );

        float indentX = lineChunks.empty() ? 0.0f : lineChunks.front().xOffset;
        float x = 0;
        size_t i = 0;

        while (i < lineText.size()) {
            FontStyle style = styleMap[i];
            size_t j = i;
            while (j < lineText.size() && styleMap[j] == style)
                ++j;

            std::string segment = lineText.substr(i, j - i);
            const char* fontFile = "chatFont.fnt";

            switch (style) {
            case Bold:       fontFile = "boldChatFont.fnt"_spr; break;
            case Italic:     fontFile = "italicChatFont.fnt"_spr; break;
            case BoldItalic: fontFile = "boldItalicChatFont.fnt"_spr; break;
            default: break;
            }

            auto lbl = CCLabelBMFont::create(segment.c_str(), fontFile);
            lbl->setAnchorPoint({ 0, 0 });
            float posY = lineY;
            if (style == Bold || style == BoldItalic || style == Italic) posY += 3.5f;
            lbl->setPosition(x + indentX, posY);
            mbf->addChild(lbl);
            x += lbl->getContentSize().width;
            i = j;
        }

        globalOffset += lineText.size();
    }
}

void RichAlertLayer::applyStrikeTags(MultilineBitmapFont* mbf, std::vector<StrikeTag> const& tags) {
    if (auto existing = mbf->getChildByID("strikelineLayer")) {
        existing->removeFromParentAndCleanup(true);
    }
    auto strikeLineLayer = CCNode::create();
    strikeLineLayer->setID("strikelineLayer");
    mbf->addChild(strikeLineLayer);

    for (const auto& tag : tags) {
        size_t globalIndex = 0;
        bool foundFirst = false;
        CCPoint leftWorld, rightWorld;
        float glyphHeight = 0;
        CCLabelBMFont* tagLabel = nullptr;

        ccColor4F strikelineColor = { 1, 1, 1, 1 };

        for (auto label : CCArrayExt<CCLabelBMFont*>(mbf->getChildren())) {
            if (!label) continue;

            auto glyphs = CCArrayExt<CCFontSprite*>(label->getChildren());
            for (auto glyph : glyphs) {
                if (globalIndex >= tag.start && globalIndex < tag.end) {
                    if (!glyph) {
                        globalIndex++;
                        continue;
                    }

                    auto pos = glyph->getPosition();
                    auto size = glyph->getContentSize();
                    auto worldPos = glyph->getParent()->convertToWorldSpace(pos);

                    float left = worldPos.x - size.width / 2;
                    float right = worldPos.x + size.width / 2;

                    if (!foundFirst) {
                        leftWorld = CCPoint(left, worldPos.y);
                        rightWorld = CCPoint(right, worldPos.y);
                        glyphHeight = size.height;
                        tagLabel = label;

                        auto col = glyph->getColor();
                        strikelineColor = {
                            col.r / 255.f,
                            col.g / 255.f,
                            col.b / 255.f,
                            1.f
                        };

                        foundFirst = true;
                    }

                    else {
                        rightWorld.x = right;
                    }
                }
                globalIndex++;
                if (globalIndex >= tag.end) break;
            }
            if (globalIndex >= tag.end) break;
        }

        if (foundFirst && tagLabel) {
            auto strikeline = CCDrawNode::create();

            auto leftLocal = mbf->convertToNodeSpace(leftWorld);
            auto rightLocal = mbf->convertToNodeSpace(rightWorld);

            float strikelineY = tagLabel->getPositionY() + 8;

            CCPoint verts[4] = {
                { leftLocal.x - 1.f, strikelineY },
                { rightLocal.x + 1.f, strikelineY },
                { rightLocal.x + 1.f, strikelineY - 0.25f },
                { leftLocal.x - 1.f, strikelineY - 0.25f },
            };

            strikeline->drawPolygon(verts, 4, strikelineColor, 0, strikelineColor);
            strikeLineLayer->addChild(strikeline);
        }
    }
}

void RichAlertLayer::applyLinkTags(MultilineBitmapFont* mbf, std::vector<LinkTag> const& links) {
    for (const auto& tag : links) {
        auto wrapper = CCNode::create();
        wrapper->setAnchorPoint({ 0, 0 });

        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = FLT_MIN, maxY = FLT_MIN;

        size_t globalIndex = 0;

        for (auto label : CCArrayExt<CCLabelBMFont*>(mbf->getChildren())) {
            if (!label) continue;

            auto glyphs = CCArrayExt<CCFontSprite*>(label->getChildren());
            for (auto glyph : glyphs) {
                if (globalIndex >= tag.start && globalIndex < tag.end) {
                    if (glyph) {
                        auto world = glyph->getParent()->convertToWorldSpace(glyph->getPosition());

                        float w = glyph->getContentSize().width * glyph->getScaleX();
                        float h = glyph->getContentSize().height * glyph->getScaleY();

                        minX = std::min(minX, world.x);
                        minY = std::min(minY, world.y);
                        maxX = std::max(maxX, world.x + w);
                        maxY = std::max(maxY, world.y + h);

                        auto clone = CCSprite::createWithTexture(glyph->getTexture(), glyph->getTextureRect());
                        clone->setAnchorPoint(glyph->getAnchorPoint());
                        clone->setScale(glyph->getScale());
                        clone->setColor(ccc3(0, 255, 255));
                        clone->setOpacity(glyph->getOpacity());

                        wrapper->addChild(clone);
                        clone->setPosition(mbf->convertToNodeSpace(world));

                        glyph->setVisible(false);
                    }
                }
                globalIndex++;
                if (globalIndex >= tag.end) break;
            }
            if (globalIndex >= tag.end) break;
        }

        auto localMin = mbf->convertToNodeSpace({ minX, minY + 55});
        auto localMax = mbf->convertToNodeSpace({ maxX, maxY + 55});

        auto width = localMax.x - localMin.x;
        auto height = localMax.y - localMin.y;

        wrapper->setContentSize({ width, height });

        for (auto child : CCArrayExt<CCNode*>(wrapper->getChildren())) {
            child->setPosition(child->getPosition() - localMin);
            child->setPositionY(child->getPositionY() + 55);
        }

        float dotSpacing = 4.0f;
        float dotSize = 2.0f;
        float yOffset = -2.0f; 

        for (float x = 0; x < width; x += dotSpacing) {
            auto dot = CCSprite::create();
            dot->setTextureRect({ 0, 0, dotSize, dotSize / 2 });
            dot->setColor(ccc3(0, 255, 255)); 
            dot->setOpacity(180);

            dot->setAnchorPoint({ 0, 1 });
            dot->setPosition({ x - 2, yOffset });
            wrapper->addChild(dot);
        }

        wrapper->setPosition({ 0, 0 });

        auto menuItem = CCMenuItemLabel::create(wrapper, this, menu_selector(RichAlertLayer::openURL));
        menuItem->setAnchorPoint({ 0, 0 });
        menuItem->setPosition(localMin);
        auto* url = new std::string(tag.url);
        menuItem->setUserData(url);

        menuItem->setAnchorPoint({ 0.5f, 0.5f });

        auto size = menuItem->getContentSize();
        menuItem->setPosition(menuItem->getPosition() + ccp(size.width * 0.5f, size.height * 0.5f));

        m_buttonMenu->addChild(menuItem);
    }
}

void RichAlertLayer::openURL(CCObject* sender) {
    if (auto item = static_cast<CCMenuItemLabel*>(sender)) {
        if (auto url = static_cast<std::string*>(item->getUserData())) {
            web::openLinkInBrowser(*url);
        }
    }
}




void RichAlertLayer::setButtonBGColor(ButtonId btn, ButtonColors col) {
    auto file = "GJ_button_01.png";

    switch (col) {
    case ButtonColors::Green:
        file = "GJ_button_01.png";
        break;
    case ButtonColors::Cyan:
        file = "GJ_button_02.png";
        break;
    case ButtonColors::Pink:
        file = "GJ_button_03.png";
        break;
    case ButtonColors::Gray:
        file = "GJ_button_04.png";
        break;
    case ButtonColors::DarkGray:
        file = "GJ_button_05.png";
        break;
    case ButtonColors::Red:
        file = "GJ_button_06.png";
        break;
    }

    if (btn == ButtonId::Btn2) {
        if (!m_button2) return;
        m_button2->updateBGImage(file);
    }
    else {
        if (!m_button1) return;
        m_button1->updateBGImage(file);
    }
}

void RichAlertLayer::addInfoButton(RichAlertLayer* popup, InfoPosition pos, float scale, CCPoint offset) {
    if (m_popup) {
        m_popup->release();
    }
    m_popup = popup;
    if (m_popup) {
        m_popup->retain();
    }

    auto spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    spr->setScale(scale);

    auto infoBtn = CCMenuItemSpriteExtra::create(
        spr,
        this,
        menu_selector(RichAlertLayer::onInfoButton)
    );

    infoBtn->setID("info-button"_spr);

    auto size = m_mainLayer->getChildByID("background")->getContentSize();

    switch (pos) {
    case InfoPosition::TopLeft:
        infoBtn->setPosition(-size.width / 2 + 25 + offset.x, size.height / 2 + 40 - 25 + offset.y);
        break;
    case InfoPosition::TopRight:
        infoBtn->setPosition(size.width / 2 - 25 + offset.x, size.height / 2 + 40 - 25 + offset.y);
        break;
    case InfoPosition::BottomLeft:
        infoBtn->setPosition(-size.width / 2 + 25 + offset.x, -size.height / 2 + 40 + 25 + offset.y);
        break;
    case InfoPosition::BottomRight:
        infoBtn->setPosition(size.width / 2 - 25 + offset.x, -size.height / 2 + 40 + 25 + offset.y);
        break;
    }

    m_buttonMenu->addChild(infoBtn);
}

RichAlertLayer::~RichAlertLayer() {
    if (m_popup) {
        m_popup->release();
        m_popup = nullptr;
    }
}
