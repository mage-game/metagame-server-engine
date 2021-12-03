
#define TIXML_USE_STL
#define TIXML_STATIC
#include "tinyxml/tinyxml.h"

#include "xmlconfig.h"

enum 
{
	OP_ROOT,
	OP_DOWN,
	OP_ATTR,
};

XMLConfig::XMLConfig()
{
	m_document = new TiXmlDocument();
}

XMLConfig::~XMLConfig()
{
	delete m_document;
}

bool XMLConfig::read(const char* file)
{
	if (m_document->LoadFile(file))
	{
		clean();
		return true;
	}
	else
	{
		return false;
	}
}
bool XMLConfig::write(const char* file)
{
	return m_document->SaveFile(file);
}

void XMLConfig::clean()
{
	m_op = OP_DOWN;
	m_current_attribute_name = "";
	m_current_node = NULL;
};
bool XMLConfig::up()
{
	if ( m_current_node == NULL ) return false;

	TiXmlNode* node = m_current_node->Parent();
	if ( node == NULL ) return false;

	return true;
}
bool XMLConfig::down()
{
	if ( m_current_node == NULL )
	{
		m_current_node = m_document;
	}
	m_op = OP_DOWN;
	return true;
}
bool XMLConfig::attribute()
{
	m_op = OP_ATTR;
	return true;
}
bool XMLConfig::value(const std::string& v, bool read_only)
{
	if ( m_op == OP_DOWN )
	{
		if ( m_current_node == NULL ) return false;
		if ( read_only )
		{
			m_current_node = m_current_node->FirstChild(v);
			if ( m_current_node == NULL ) return false;

			return true;	
		}
		else
		{
			TiXmlNode* node = m_current_node->FirstChild(v);
			if ( node == NULL )
			{
				TiXmlElement element(v);
				node = m_current_node->InsertEndChild(element);
				if ( node == NULL )
				{
					return false;
				}
			}
			m_current_node = node;
			return true;
		}
	}
	else if ( m_op == OP_ATTR )
	{
		if ( m_current_node == NULL ) return false;
		m_current_attribute_name = v;
		return true;
	}
	else
	{
		return false;
	}
}

bool XMLConfig::list(std::vector<std::string>* element_list, std::vector<std::string>* attribute_list)
{
	if ( m_op == OP_DOWN )
	{
		if ( m_current_node == NULL ) return false;
		
		if ( element_list != NULL )
		{
			for( TiXmlElement* child = m_current_node->FirstChildElement(); child != NULL; child = child->NextSiblingElement() )
			{
				element_list->push_back(child->ValueStr());
			}
		}

		TiXmlElement* element = m_current_node->ToElement();
		if ( attribute_list != NULL && element != NULL)
		{
			for( TiXmlAttribute* attr = element->FirstAttribute(); attr != NULL; attr = attr->Next() )
			{
				element_list->push_back(attr->Name());
			}
		}
	
		return true;
	}
	else if ( m_op == OP_ATTR )
	{
		return false;
	}
	else
	{
		return false;
	}
}

bool XMLConfig::get(std::string* str)const
{
	if ( m_op == OP_DOWN )
	{
		if ( m_current_node == NULL ) return false;
		TiXmlElement* element = m_current_node->ToElement();
		if ( element == NULL ) return false;
		const char* text = element->GetText();
		if ( text == NULL ) return false;

		*str = text;
		return true;
	}
	else if ( m_op == OP_ATTR )
	{
		if ( m_current_node == NULL ) return false;
		TiXmlElement* element = m_current_node->ToElement();
		if ( element == NULL ) return false;

		const std::string* attribute = element->Attribute(m_current_attribute_name);
		if ( attribute == NULL ) return false;

		*str = *attribute;
		return true;
	}
	else
	{
		return false;
	}
};
bool XMLConfig::set(const std::string& str)
{
	if ( m_op == OP_DOWN )
	{
		if ( m_current_node == NULL ) return false;

		TiXmlNode * find_node = dynamic_cast<TiXmlText*>(m_current_node->FirstChild());
		if (find_node != NULL)
		{
			find_node->SetValue(str);
			return true;
		}

		TiXmlText text(str);
		TiXmlNode* node = m_current_node->InsertEndChild(text);
		if ( node == NULL ) return false;

		return true;
	}
	else if ( m_op == OP_ATTR )
	{
		if ( m_current_node == NULL ) return false;
		TiXmlElement* element = m_current_node->ToElement();
		if ( element == NULL ) return false;
		element->SetAttribute(m_current_attribute_name, str);
		return true;
	}
	else
	{
		return false;
	}
}

