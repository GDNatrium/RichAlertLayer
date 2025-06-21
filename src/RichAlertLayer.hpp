#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

enum class ButtonId {
    Btn1,
    Btn2
};

enum class InfoPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class ButtonColors {
    Green,
    Cyan,
    Pink,
    Gray,
    DarkGray,
    Red
};

enum FontStyle { Normal, Bold, Italic, BoldItalic };

class RichAlertLayer : public FLAlertLayer {
private:
    ~RichAlertLayer();
    RichAlertLayer* m_popup = nullptr;

    struct ColorTag {
        size_t start;
        size_t end;
        cocos2d::ccColor3B color;
    };

    struct UnderlineTag {
        size_t start;
        size_t end;
    };

    struct BoldTag {
        size_t start;
        size_t end;
    };

    struct ItalicTag {
        size_t start;
        size_t end;
    };

    struct StrikeTag {
        size_t start;
        size_t end;
    };

    struct LinkTag {
        size_t start;
        size_t end;
        std::string url;
    };

    struct ParsedText {
        std::string text;
        std::vector<ColorTag> colors;
        std::vector<UnderlineTag> underlines;
        std::vector<BoldTag> boldTags;
        std::vector<ItalicTag> italicTags;
        std::vector<StrikeTag> strikeTags;
        std::vector<LinkTag> links;
    };


    static ParsedText parseRichText(std::string const& raw);
    void applyColorTags(MultilineBitmapFont* mbf, std::vector<ColorTag> const& tags);
    void applyUnderlineTags(MultilineBitmapFont* mbf, std::vector<UnderlineTag> const& tags);
    void applyFontStyleTags(MultilineBitmapFont* mbf,
        std::vector<BoldTag> const& boldTags,
        std::vector<ItalicTag> const& italicTags);
    void applyStrikeTags(MultilineBitmapFont* mbf, std::vector<StrikeTag> const& tags);
    void applyLinkTags(
        MultilineBitmapFont* mbf,
        std::vector<LinkTag> const& links
    );


public:
    static RichAlertLayer* create(
        std::string const& title,
        std::string const& desc,
        std::string const& btn1,
        std::string const& btn2 = "",
        float width = 300.f,
        bool scroll = false,
        float height = 140.f,
        float textScale = 1.f
    );

    void show();

    void setButtonBGColor(ButtonId btn, ButtonColors color);

    void addInfoButton(RichAlertLayer* popup, InfoPosition pos, float scale = 1.f, CCPoint offset = { 0,0 });

    void setPopup(RichAlertLayer* popup) {
        m_popup = popup;
    }

    void onInfoButton(CCObject* sender) {
        if (m_popup) {
            m_popup->show();
        }
    }

    void openURL(CCObject* sender);

    static std::vector<FontStyle> buildStyleMap(
        size_t textLen,
        std::vector<BoldTag>   const& boldTags,
        std::vector<ItalicTag> const& italicTags
    ) {
        std::vector<FontStyle> map(textLen, Normal);

        for (auto const& t : boldTags) {
            if (t.end > textLen)
                log::warn("BoldTag range [{}-{}) exceeds text length {}", t.start, t.end, textLen);
            for (size_t i = t.start; i < t.end && i < textLen; ++i)
                map[i] = Bold;
        }

        for (auto const& t : italicTags) {
            if (t.end > textLen)
                log::warn("ItalicTag range [{}-{}) exceeds text length {}", t.start, t.end, textLen);
            for (size_t i = t.start; i < t.end && i < textLen; ++i)
                map[i] = (map[i] == Bold) ? BoldItalic : Italic;
        }

        return map;
    }


protected:
    std::string m_desc;
    CCNode* m_richNode = nullptr;

    bool init(
        std::string const& title,
        std::string const& richText,
        std::string const& btn1,
        std::string const& btn2,
        float width,
        bool scroll,
        float height,
        float textScale
    );
};


