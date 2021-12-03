
#include "rmibase.h"
#include "rmidef.h"

namespace rmi
{
	int RMIObject::__dispatch(const char *method, TLVUnserializer &in_param, TLVSerializer *out_param)
	{
		MethodList::iterator i = m_method_list.find(method);
		if (i != m_method_list.end())
		{
			return i->second(in_param, out_param);
		}
		return DispatchMethodNotExist;
	}
}

