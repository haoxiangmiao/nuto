#pragma once

namespace NuTo
{

enum class eShape
{
    Line,
    Triangle,
    Quadrilateral,
    Tetrahedron,
    Hexahedron,
    Prism,
    Pyramid
};

class Shape
{
public:
    virtual eShape Enum() const = 0;
};

} // namespace NuTo
