
#include "tlvprotocol.h"

/*
	MakeTLV 和 PopTLV 两个函数有严重的代码重复，下一次一旦需要修改这两个函数时，需先解决此问题

*/
#if defined(__GNUC_VA_LIST)
const char* MakeTLV(TLVSerializer* tlv, const char* format, va_list v)
#else
const char* MakeTLV(TLVSerializer* tlv, const char* format, va_list v, void **v_out)
#endif
{
	const char* p = format;
	for(; *p != 0; ++p)
	{
		switch(*p)
		{
		case 'b':
			{
				unsigned char c = va_arg(v, int);		//整型数据，均需要使用int转型接收， 参见ISO Sec. 6.3.2.2
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 'h':
			{
				short c = va_arg(v, int);
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 'i':
			{
				int c = va_arg(v, int);
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 'k':
			{
				long long c = va_arg(v, long long);
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 'f':
			{
				float c = (float)va_arg(v, double);	//同理，浮点数需要用double接收
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 'd':
			{
				double c = va_arg(v, double);
				if ( !tlv->Push(c) )
				{
					return 0;
				}
			}
			break;
		case 's':
			{
				const char *str = va_arg(v, const char*);
				if ( !tlv->Push(str) )
				{
					return 0;
				}
			}
			break;
		case '[':
			{
				TLVSerializer subtlv;
				char *cur_buffer = (char*)tlv->Ptr();
				unsigned int use_size = tlv->Size() + sizeof(TLVType_t) + sizeof(TLVLength_t);
				subtlv.Reset(cur_buffer + use_size, tlv->MaxSize() - use_size);

		#if defined(__GNUC_VA_LIST)
				p = MakeTLV(&subtlv, p+1, v);
		#else
				void *v_out = 0;
				p = MakeTLV(&subtlv, p+1, v, &v_out);
				v = (va_list)v_out;
		#endif

				
				if ( p == 0 || *p != ']')
				{
					return 0;
				}
				else
				{
					// 这里用比较难看的做法做到嵌套组包不重复复制
					cur_buffer += tlv->Size();
					*(TLVType_t*)(cur_buffer) = (TLVType_t)TLVTypeTraits<TLVSerializer>::TypeId;
					cur_buffer = (char*)cur_buffer + sizeof(TLVType_t);

					// 非定长，必须写 Length
					*(TLVLength_t*)(cur_buffer) = (TLVLength_t)subtlv.Size();
					cur_buffer = (char*)cur_buffer + sizeof(TLVLength_t);

					tlv->MoveCurPos(sizeof(TLVType_t) + sizeof(TLVLength_t) + subtlv.Size());
				}
			}
			break;
		case ']':
			{

			#if defined(__GNUC_VA_LIST)

			#else
				*v_out = (void *)v;
			#endif
				return p;
			}
		case 'l':
			{
				TLVSerializer *s = va_arg(v, TLVSerializer*);
				if ( !tlv->Push(*s) )
				{
					return 0;
				}
			}
			break;
		default:
			return 0;

		}
	}

#if defined(__GNUC_VA_LIST)

#else
	*v_out = (void *)v;
#endif

	return p;
}

#if defined(__GNUC_VA_LIST)
const char* PopTLV(TLVUnserializer* tlv, const char* format, va_list v)
#else
const char* PopTLV(TLVUnserializer* tlv, const char* format, va_list v, void **v_out)
#endif
{
	const char* p = format;
	for(; *p != 0; ++p)
	{
		switch(*p)
		{
		case 'b':
			{
				unsigned char *c = va_arg(v, unsigned char*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 'h':
			{
				short *c = va_arg(v, short*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 'i':
			{
				int *c = va_arg(v, int*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 'k':
			{
				long long *c = va_arg(v, long long*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 'f':
			{
				float *c = va_arg(v, float*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 'd':
			{
				double *c = va_arg(v, double*);
				if ( !tlv->Pop(c) )
				{
					return 0;
				}
			}
			break;
		case 's':
			{
				const char **str = va_arg(v, const char**);
				if ( !tlv->Pop(str) )
				{
					return 0;
				}
			}
			break;
		case '[':
			{
				TLVUnserializer subtlv;
				if ( !tlv->Pop(&subtlv) )
				{
					return 0;
				}

			#if defined(__GNUC_VA_LIST)
				p = PopTLV(&subtlv, p+1, v);
			#else
				void *v_out = 0;
				p = PopTLV(&subtlv, p+1, v, &v_out);
				v = (va_list)v_out;
			#endif

				if ( p == 0 || *p != ']')
				{
					return 0;
				}
			}
			break;
		case ']':
			{

			#if defined(__GNUC_VA_LIST)

			#else
				*v_out = (void *)v;
			#endif

				return p;
			}
			break;
		case 'l':
			{
				TLVUnserializer *s = va_arg(v, TLVUnserializer*);
				if ( !tlv->Pop(s) )
				{
					return 0;
				}
			}
			break;
		default:
			return 0;

		}
	}

#if defined(__GNUC_VA_LIST)

#else
	*v_out = (void *)v;
#endif

	return p;
}


bool TLVSerializer::Pushf(const char* format, ...)
{
	va_list v;
	va_start(v, format);
	bool ret = Pushv(format, v);
	va_end(v);
	return ret;
}
bool TLVSerializer::Pushv(const char* format, va_list v)
{

#if defined(__GNUC_VA_LIST)
	const char* p = MakeTLV(this, format, v);
#else
	void *v_out = 0;
	const char* p = MakeTLV(this, format, v, &v_out);
#endif
	
	return p != 0;
}

bool TLVUnserializer::Popf(const char* format, ...)
{
	va_list v;
	va_start(v, format);
	bool ret = Popv(format, v);
	va_end(v);
	return ret;
}

bool TLVUnserializer::Popv(const char* format, va_list v)
{

#if defined(__GNUC_VA_LIST)
	const char* p = PopTLV(this, format, v);
#else
	void *v_out = 0;
	const char* p = PopTLV(this, format, v, &v_out);
#endif

	return p != 0;
}
