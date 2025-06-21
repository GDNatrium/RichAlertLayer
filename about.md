# RichAlertLayer

<cr>This mod has very little functionality on it's own. It is intended for mod developers.</c>

---

RichAlertLayer is an expansion of FLAlertLayer, which adds a lot of new functionality for text styling,
and some QOL functions for styling the popup itself.  
  
## Usage
You can use it by including the "RichAlertLayer" header file.
```
#include "RichAlertLayer.hpp"
```
You can now create FLAlertLayers like normal, but using <cg>RichAlertLayer</c>::<cy>create</c> instead.
```
auto alert = RichAlertLayer::create(
	"Hello World",
	"RichAlertLayer mod!",
	"OK"
);
alert->show();
```

---

## New Functionality
All of the new styles can be combined and stacked ontop of eachother, with even complex combinations working well.

### More Colors
You are not limited to pre-defined color tags anymore. You can now change any part of the description to any color.
RichAlertLayer adds color tags which take hex code, and apply color according to them.   
```
"My <col=#ff5500>custom color</col> text."
```

---

### Bold Text
Description text can be bold now. Can be combined with italic tags for bold-italic text.  
```
"My <b>bold</b> text."
```

---

### Italic Text
Description text can be italic now. Can be combined with bold tags for bold-italic text.  
```
"My <i>italic</i> text."
```

---

### Strikthrough Text
Description text can be strike-through now.
```
"My <s>strike-through</s> text."
```

---

### Underline Text
Description text can be underlined now.   
```
"My <u>underlined</u> text."
```

---

### Easy Link Integration
Description text can now contain links, which will be opened in the browser on click.
Links have some default formatting, which can be overridden by other tags.  
```
"Click <link=https://www.google.com/>here</link>!"
```

---

### Changing Button Colors More Easily
RichAlertLayer adds a new function <cy>setButtonBGColor</c> for changing the background of buttons more efficiently.
You can now select from an enum of buttons and colors, instead of inputting the file name.  
<cy>setButtonBGColor</c>(<cg>ButtonId</c> btn, <cg>ButtonColors</c> col);
- ButtonId: Btn1, Btn2
- ButtonColors: Green, Cyan, Pink, Gray, DarkGray, Red
```
alert->setButtonBGColor(
ButtonId::Btn1, 
ButtonColors::Red
);
```

---

### Easily adding Info Popups
You can now easily add an info button, along with a custom help popup to your AlertLayer with just one function.
<cy>addInfoButton</c> does all the sprite creation, positioning and menu selector management for you.  
<cy>addInfoButton</c>(<cg>RichAlertLayer*</c> popup, <cg>InfoPosition*</c> pos, <cb>float</c> float, <cg>CCPoint</c> offset);
- InfoPosition: TopLeft, TopRight, BottomLeft, BottomRight
```
auto info = RichAlertLayer::create(
"Help",
"This is the help text.",
"OK"
)

alert->addInfoButton(info, InfoPosition::TopRight);
```
