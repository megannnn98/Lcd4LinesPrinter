#include <cstdint>
#include <string>
#include <array>
#include <iostream>
#include <any>

class LcdDataHolder
{
public:
    static const int LINES = 4;
    static const int CHARS = 20;

    enum TypeCode: std::uint8_t
    {
        eFLOAT, 
        eINT, 
        eBOOL, 
    };
private:
    struct LcdLine {
        TypeCode code;
        std::string_view name;
        std::any data;
        std::string_view unit;
    };

    using LcdBuffer = std::array<std::array<std::uint8_t, CHARS>, LINES>;
    LcdBuffer m_buffer{};
    LcdLine m_lines[LINES];
    LcdDataHolder(const LcdDataHolder&) = delete;
    LcdDataHolder(LcdDataHolder&&) = delete;
    LcdDataHolder& operator=(const LcdDataHolder&) = delete;
    LcdDataHolder& operator=(LcdDataHolder&&) = delete;

    template <typename T>
    void m_MakeLine(char* out, const LcdLine& line)
    {
        if (std::is_same_v<float, T>)
            std::snprintf(out, CHARS, "%.6s %.2f %s", 
                line.name.data(), std::any_cast<float>(line.data), line.unit.data());
        else if (std::is_same_v<bool, T>)
            std::snprintf(out, CHARS, "%.5s %1d %s", 
                line.name.data(), std::any_cast<T>(line.data), line.unit.data());
        else 
            std::snprintf(out, CHARS, "%.5s %.1d %s", 
                line.name.data(), std::any_cast<T>(line.data), line.unit.data());
    }

public:
    LcdBuffer GetBuffer() const 
    {
        return m_buffer;
    }

    void CommitDataToBuffer()
    {
        int i = 0;
        for(const auto& line: m_lines)
        {
            if (!line.data.has_value())
                continue;
            char* buffer = reinterpret_cast<char*>(m_buffer[i].begin());
            switch(line.code)
            {
                case eFLOAT:
                    m_MakeLine<float>(buffer, line);
                break;
                case eINT:
                    m_MakeLine<std::int32_t>(buffer, line);
                break;
                case eBOOL:
                    m_MakeLine<bool>(buffer, line);
                break;
            }
            ++i;
        }
    }

    template <typename T>
    void SetLine(std::uint8_t num, std::string_view name, T value, std::string_view unit)
    {
        static_assert((std::is_same_v<T, float>) || 
                        (std::is_same_v<T, std::int32_t>) || 
                            (std::is_same_v<T, bool>), "Wrong type passed!!");

        m_lines[num].code = (std::is_same_v<T, float>) ? TypeCode::eFLOAT :
                                (std::is_same_v<T, std::int32_t>) ? TypeCode::eINT :
                                    TypeCode::eBOOL;
        m_lines[num].name = name;
        m_lines[num].data = value;
        m_lines[num].unit = unit;
    }

    ~LcdDataHolder() = default;    
    LcdDataHolder() = default;    

};

int main()
{
    LcdDataHolder ldh;

    ldh.SetLine(0, "distance", 123, "km");
    ldh.SetLine(1, "distance", 7.89f, "km");
    ldh.SetLine(2, "distance", true, "km");
    
    ldh.CommitDataToBuffer();

    std::cout << "-------------------" << '\n';
    for (int i = 0; i < LcdDataHolder::LINES; ++i)
    {
        for (int j = 0; j < LcdDataHolder::CHARS; ++j)
            std::cout << ldh.GetBuffer()[i][j];
        std::cout << "\n";
    }

    return 0;
}
