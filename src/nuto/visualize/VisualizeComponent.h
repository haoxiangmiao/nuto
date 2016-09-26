// $Id$ 
#pragma once
#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include <string>

namespace NuTo
{

enum class eVisualizeWhat;

class VisualizeComponent
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    VisualizeComponent(NuTo::eVisualizeWhat rVisualizeComponent);

    virtual ~VisualizeComponent()= default;

    virtual NuTo::eVisualizeWhat GetComponentEnum()const;

    virtual std::string GetComponentName()const;

    virtual int GetElementId()const;

    virtual int GetIp()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

protected:
    //! @brief just for serialization
    VisualizeComponent() = default;

private:
    eVisualizeWhat mVisualizeComponent;

};
}

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::VisualizeComponent)
#endif // ENABLE_SERIALIZATION

