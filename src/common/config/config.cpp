
#include "config.h"

const char* next_word(const char* path, std::string* word)
{
	word->clear();
	const char* ptr = path;
	while(*ptr != '\0')
	{
		switch(*ptr)
		{
		case '\\':
			word->push_back(*(++ptr));
			++ptr;
			break;
		case '/':
			if (word->empty())
			{
				*word = "/";
				return ++ptr;
			}
			else
			{
				return ptr;
			}
		case '@':
			if (word->empty())
			{
				*word = "@";
				return ++ptr;
			}
			else
			{
				return ptr;
			}
		default:
			word->push_back(*ptr++);
			break;
		}
	}
	return ptr;
}

bool IConfig::get_item(const char* path, std::string* val)const
{
	((IConfig*)this)->clean();
	if (((IConfig*)this)->find_item(path, true) && get(val))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool IConfig::set_item(const char* path, const std::string& val)
{
	clean();
	find_item(path, false);

	if (!set(val)) return false;

	return true;
}

bool IConfig::list_item(const char* path, std::vector<std::string>* element_list, std::vector<std::string>* attribute_list)
{
	clean();
	find_item(path, true);

	if (!list(element_list, attribute_list)) return false;

	return true;
}

bool IConfig::find_item(const char* path, bool read_only)
{
	const char* ptr = path;

	while(*ptr != '\0')
	{
		std::string word;
		ptr = next_word(ptr, &word);

		if (word == "/")
		{
			if (!down()) return false;
		}
		else if ( word == ".." )
		{
			if (!up()) return false;
		}
		else if (word == "@")
		{
			if (!attribute()) return false;
		}
		else
		{
			if (!value(word, read_only)) return false;
		}
	}

	return true;
}
