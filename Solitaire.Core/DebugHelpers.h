#pragma once

namespace Debug
{
    inline void OutputDebugStringStream(std::wstringstream& stringStream)
    {
        std::wstring token;
        while (std::getline(stringStream, token))
        {
            OutputDebugStringW(token.c_str());
            OutputDebugStringW(L"\n");
        }
    }

    template <typename T>
    inline void PrintProperty(std::wstring const& propertyName, T const value, T const defaultValue, std::wstringstream& stream, int indent)
    {
        if (value != defaultValue)
        {
            PrintProperty(propertyName, value, stream, indent);
        }
    }

    template <typename T>
    inline void PrintProperty(std::wstring const& propertyName, T const value, std::wstringstream& stream, int indent)
    {
        AddIndents(stream, indent + 1);
        stream << propertyName.c_str();
        PrintValue(value, stream);
        stream << std::endl;
    }

    inline void PrintValue(winrt::Windows::Foundation::Numerics::float2 const value, std::wstringstream& stream)
    {
        stream << L"{ " << value.x << L", " << value.y << L" }";
    }

    inline void PrintValue(winrt::Windows::Foundation::Numerics::float3 const value, std::wstringstream& stream)
    {
        stream << L"{ " << value.x << L", " << value.y << L", " << value.z << L" }";
    }

    inline void PrintValue(float const value, std::wstringstream& stream)
    {
        stream << value;
    }

    inline void AddIndents(std::wstringstream& stream, int indent)
    {
        for (auto i = 0; i < indent; i++)
        {
            stream << L"    ";
        }
    }

    inline void PrintVisual(winrt::Windows::UI::Composition::Visual const& visual, std::wstringstream& stream, int indent)
    {
        AddIndents(stream, indent);
        auto name = visual.Comment();
        if (name.empty())
        {
            name = L"(Nameless Visual)";
        }
        stream << name.c_str() << std::endl;
        PrintProperty(L"Size: ", visual.Size(), { 0, 0 }, stream, indent + 1);
        PrintProperty(L"RelativeSize: ", visual.RelativeSizeAdjustment(), { 0, 0 }, stream, indent + 1);
        PrintProperty(L"Offset: ", visual.Offset(), { 0, 0, 0 }, stream, indent + 1);
        PrintProperty(L"RelativeOffset: ", visual.RelativeOffsetAdjustment(), { 0, 0, 0 }, stream, indent + 1);
        PrintProperty(L"RotationAxis: ", visual.RotationAxis(), { 0, 0, 1 }, stream, indent + 1);
        PrintProperty(L"RotationAngleInDegrees: ", visual.RotationAngleInDegrees(), 0.0f, stream, indent + 1);
    }

    inline void PrintTree(winrt::Windows::UI::Composition::Visual const& root, std::wstringstream& stream, int indent)
    {
        PrintVisual(root, stream, indent);
        auto containerVisual = root.try_as<winrt::Windows::UI::Composition::ContainerVisual>();
        if (containerVisual)
        {
            if (containerVisual.Children().Count() > 0)
            {
                AddIndents(stream, indent + 1);
                stream << L"Children:" << std::endl;
                for (auto& child : containerVisual.Children())
                {
                    PrintTree(child, stream, indent + 2);
                }
            }
        }
    }
}
