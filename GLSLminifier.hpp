#include <string>
#include <array>


[[nodiscard]] std::string GLSLminifier(std::string ShaderSourc)
{
    // Remove Backslash
    std::erase_if(ShaderSourc, [](char c)
                  {
        static bool skip_next = false;
        if (skip_next) {
            skip_next = false;
            return true;   
        }
        if (c == '\\') {
            skip_next = true;
            return true;   
        }
        return false; });

    // Preprocessor directive protection: if the collected run contained a newline and either the ShaderSourc was already inside a # directive (tracked via a flag set the moment a # is written) or the character immediately following the run is # then the entire run collapses to exactly one \n. This guarantees a #define or similar directive always starts on its line and never gets glued to the previous lines code.

    // Symbol-adjacency rule: otherwise if either side of the run is a Symbol. Meaning the pair is Symbol↔Symbol, Symbol↔Digit, Digit↔Symbol, Symbol↔Word or Word↔Symbol. The entire run is deleted with no space left behind all. This is what makes x = 3 collapse to x=3 and # 100 collapse to #100.

    // Default single-space rule: if neither of the applies but there is a real character, on both sides (Word↔Word, Word↔Digit, Digit↔Word or Digit↔Digit) the whole run. Regardless of how many spaces, tabs or newlines it contained. Is replaced by exactly one space character.

    // Edge case: if the run occurs at the start or very end of the text (one side has no character, i.e. None) its simply dropped entirely no space added.
    enum class CharClass : unsigned char
    {
        None,
        Word,
        Digit,
        Symbol
    };

    static constexpr std::array<CharClass, 256> kClassTable = []
    {
        std::array<CharClass, 256> t{};
        for (int i = 0; i < 256; ++i)
        {
            if ((i >= 'a' && i <= 'z') || (i >= 'A' && i <= 'Z') || i >= 0x80)
                t[static_cast<std::size_t>(i)] = CharClass::Word; // >=0x80: UTF-8 
            else if (i >= '0' && i <= '9')
                t[static_cast<std::size_t>(i)] = CharClass::Digit;
            else
                t[static_cast<std::size_t>(i)] = CharClass::Symbol; // Including #
        }
        return t;
    }();

    constexpr auto isWs = [](unsigned char c) constexpr noexcept
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    };

    std::string out;
    out.reserve(ShaderSourc.size());

    const std::size_t n = ShaderSourc.size();
    std::size_t i = 0;
    CharClass lastClass = CharClass::None;
    bool inDirective = false;

    while (i < n)
    {
        const unsigned char c = static_cast<unsigned char>(ShaderSourc[i]);
        const bool isLineComment = (c == '/' && i + 1 < n && ShaderSourc[i + 1] == '/');
        const bool isBlockComment = (c == '/' && i + 1 < n && ShaderSourc[i + 1] == '*');

        if (!isWs(c) && !isLineComment && !isBlockComment)
        {
            out.push_back(static_cast<char>(c));
            if (c == '#')
                inDirective = true;
            lastClass = kClassTable[c];
            ++i;
            continue;
        }

        // Batch processing as a unit: Spaces + Tabs + Line breaks + Comments // and /*...*/
        std::size_t j = i;
        bool hasNewline = false;
        while (j < n)
        {
            const unsigned char wc = static_cast<unsigned char>(ShaderSourc[j]);

            if (isWs(wc))
            {
                hasNewline |= (wc == '\n' || wc == '\r');
                ++j;
                continue;
            }
            if (wc == '/' && j + 1 < n && ShaderSourc[j + 1] == '/')
            {
                j += 2;
                while (j < n && ShaderSourc[j] != '\n' && ShaderSourc[j] != '\r')
                    ++j;
                continue; // The line will return to me after the comment; it will be read as whitespace in the next iteration.
            }
            if (wc == '/' && j + 1 < n && ShaderSourc[j + 1] == '*')
            {
                j += 2;
                while (j < n)
                {
                    const unsigned char bc = static_cast<unsigned char>(ShaderSourc[j]);
                    if (bc == '\n' || bc == '\r')
                        hasNewline = true;
                    if (bc == '*' && j + 1 < n && ShaderSourc[j + 1] == '/')
                    {
                        j += 2;
                        break;
                    }
                    ++j;
                }
                continue;
            }
            break; // No distance, no comment -> We stopped
        }

        const bool nextIsHash = (j < n) && (ShaderSourc[j] == '#');

        if (hasNewline && (inDirective || nextIsHash))
        {
            out.push_back('\n');
            inDirective = false;
            i = j;
            continue;
        }

        const CharClass nextClass = (j < n) ? kClassTable[static_cast<unsigned char>(ShaderSourc[j])] : CharClass::None;
        const bool removeGap =
            (lastClass == CharClass::Symbol && nextClass == CharClass::Symbol) ||
            (lastClass == CharClass::Symbol && nextClass == CharClass::Digit) ||
            (lastClass == CharClass::Digit && nextClass == CharClass::Symbol) ||
            (lastClass == CharClass::Symbol && nextClass == CharClass::Word) ||
            (lastClass == CharClass::Word && nextClass == CharClass::Symbol);

        if (lastClass != CharClass::None && nextClass != CharClass::None && !removeGap)
        {
            out.push_back(' ');
        }

        i = j;
    }

    return out;
}