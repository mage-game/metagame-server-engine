
#include <assert.h>
#include <string>
#include "rmidef.h"

namespace rmi
{
	


	RMICall::RMICall()
	{

	}
	RMICall::~RMICall()
	{

	}

	bool RMICall::Serialize(TLVSerializer *s, const TLVSerializer &in_param)
	{
		return s->Pushf("issl", message_id, module, method, &in_param);
	}
	bool RMICall::UnSerialize(TLVUnserializer &s, TLVUnserializer *in_param)
	{
		return s.Popf("issl", &message_id, &module, &method, in_param);
	}


	RMIReturn::RMIReturn()
	{

	}
	RMIReturn::~RMIReturn()
	{

	}
	bool RMIReturn::Serialize(TLVSerializer *s, const TLVSerializer &out_param)
	{
		return s->Pushf("ibl", message_id, call_result, &out_param);
	}
	bool RMIReturn::UnSerialize(TLVUnserializer &s, TLVUnserializer *out_param)
	{
		return s.Popf("ibl", &message_id, &call_result, out_param);
	}


}


