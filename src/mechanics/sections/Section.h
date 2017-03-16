#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include <cfloat>

namespace NuTo
{
 
//! @brief Abstract base class for sections
class Section
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    //! @brief Destructor
    virtual ~Section() = 0;

    //! @brief Print information about the section
    virtual void Info() const;

    //! @brief Get the cross-section area of a 1D section
    //! @param coordinate Global coordinate at which to get the area
    virtual double GetArea(double coordinate = DBL_MIN) const;

    //! @brief Get the thickness of a 2D section
    virtual double GetThickness() const;

    //! @brief Get the circumference of a fibre section
    virtual double GetCircumference() const;

    //! @brief Check if section is plane strain (if false, it's plane stress)
    virtual bool IsPlaneStrain() const;
#ifdef ENABLE_SERIALIZATION
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
    }

    BOOST_CLASS_EXPORT_KEY(NuTo::SectionBase)
#endif // ENABLE_SERIALIZATION
};

} // namespace NuTo
