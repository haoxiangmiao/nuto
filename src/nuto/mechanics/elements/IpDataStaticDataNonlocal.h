// $ld: $ 
#ifndef IPDATASTATICDATANONLOCAL_H_
#define IPDATASTATICDATANONLOCAL_H_

#include "nuto/mechanics/elements/IpDataStaticDataBase.h"
#include "nuto/mechanics/elements/IpDataNonlocalBase.h"

namespace NuTo
{
//! @author Joerg F. Unger
//! @date Apr 28, 2010
//! @brief ...
class IpDataStaticDataNonlocal : public IpDataStaticDataBase ,public IpDataNonlocalBase
{
public:
	IpDataStaticDataNonlocal();

	virtual ~IpDataStaticDataNonlocal();

	virtual void Initialize(const ElementWithDataBase* rElement, const ConstitutiveBase* rConstitutive);
};
}
#endif /* IPDATASTATICDATANONLOCAL_H_ */