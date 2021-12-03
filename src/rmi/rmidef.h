
#ifndef RMIOBJECT_H
#define RMIOBJECT_H

#include "common/tlvprotocol.h"
#include "rmibase.h"

namespace rmi
{
	class RMICall
	{
	public:
		RMICall();
		~RMICall();

		int	message_id;
		const char		*module;
		const char		*method;

		unsigned int GetSerializeLength(const TLVSerializer &in_param) { return in_param.Size() + 20 + MAX_IDENTITY_LEN * 2;}
		bool Serialize(TLVSerializer *s, const TLVSerializer &in_param);
		bool UnSerialize(TLVUnserializer &s, TLVUnserializer *in_param);
	};

	// ∑µªÿ∂‘œÛ
	class RMIReturn
	{
	public:
		RMIReturn();
		~RMIReturn();

		int				message_id;
		char			call_result;

		unsigned int GetSerializeLength(const TLVSerializer &out_param) { return out_param.Size() + 12;}
		bool Serialize(TLVSerializer *s, const TLVSerializer &out_param);
		bool UnSerialize(TLVUnserializer &s, TLVUnserializer *out_param);
	};
}



#endif

