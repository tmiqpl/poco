//
// CDATASection.cpp
//
// Library: XML
// Package: DOM
// Module:  DOM
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "Poco/DOM/CDATASection.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/DOMException.h"


namespace Poco {
namespace XML {


const XMLString CDATASection::NODE_NAME = toXMLString("#cdata-section");


CDATASection::CDATASection(Document::Ptr pOwnerDocument, const XMLString& data):
	Text(pOwnerDocument, data)
{
}


CDATASection::CDATASection(Document::Ptr pOwnerDocument, const CDATASection& sec):
	Text(pOwnerDocument, sec)
{
}


CDATASection::~CDATASection()
{
}


Text::Ptr CDATASection::splitText(unsigned long offset)
{
	Node::Ptr pParent = parentNode();
	if (!pParent) throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	int n = length() - offset;
	Text::Ptr pNew = ownerDocument()->createCDATASection(substringData(offset, n));
	deleteData(offset, n);
	pParent->insertBefore(pNew, nextSibling());
	return pNew;
}


const XMLString& CDATASection::nodeName() const
{
	return NODE_NAME;
}


unsigned short CDATASection::nodeType() const
{
	return Node::CDATA_SECTION_NODE;
}


Node::Ptr CDATASection::copyNode(bool deep, Document::Ptr pOwnerDocument) const
{
	return new CDATASection(pOwnerDocument, *this);
}


} } // namespace Poco::XML
