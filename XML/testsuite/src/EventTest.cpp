//
// EventTest.cpp
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include "EventTest.h"
#include "Poco/CppUnit/TestCaller.h"
#include "Poco/CppUnit/TestSuite.h"
#include "Poco/DOM/Event.h"
#include "Poco/DOM/MutationEvent.h"
#include "Poco/DOM/EventListener.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/Text.h"
#include "Poco/RefPtr.h"


using Poco::XML::Event;
using Poco::XML::MutationEvent;
using Poco::XML::EventListener;
using Poco::XML::Element;
using Poco::XML::Document;
using Poco::XML::Text;
using Poco::XML::Node;
using Poco::XML::XMLString;
using Poco::RefPtr;


class TestEventListener: public EventListener
{
public:
	typedef RefPtr<TestEventListener> Ptr;

	TestEventListener(const XMLString& name, bool cancel = false, bool readd = false, bool capture = false):
		_name(name),
		_cancel(cancel),
		_readd(readd),
		_capture(capture)
	{
	}
	
	void handleEvent(Event::Ptr evt)
	{
		XMLString type = evt->type();
		XMLString phase;
		switch (evt->eventPhase())
		{
		case Event::CAPTURING_PHASE:
			phase = "CAPTURING_PHASE"; break;
		case Event::AT_TARGET:
			phase = "AT_TARGET"; break;
		case Event::BUBBLING_PHASE:
			phase = "BUBBLING_PHASE"; break;
		}
		Node::Ptr pTarget = evt->target().cast<Node>();
		Node::Ptr pCurrentTarget = evt->currentTarget().cast<Node>();

		_log.append(_name);
		_log.append(":");
		_log.append(type);
		_log.append(":");
		_log.append(phase);
		_log.append(":");
		_log.append(pTarget->nodeName());
		_log.append(":");
		_log.append(pCurrentTarget->nodeName());
		_log.append(":");
		_log.append(evt->bubbles() ? "B" : "-");
		_log.append(":");
		_log.append(evt->cancelable() ? "C" : "-");

		MutationEvent::Ptr pME = evt.cast<MutationEvent>();
		if (pME)
		{
			XMLString attrChange;
			switch (pME->attrChange())
			{
			case MutationEvent::MODIFICATION:
				attrChange = "MODIFICATION"; break;
			case MutationEvent::ADDITION:
				attrChange = "ADDITION"; break;
			case MutationEvent::REMOVAL:
				attrChange = "REMOVAL"; break;
			}
			XMLString relatedNode;
			Node::Ptr pRelatedNode = pME->relatedNode();
			if (pRelatedNode) relatedNode = pRelatedNode->nodeName();

			_log.append(":");
			_log.append(attrChange);
			_log.append(":");
			_log.append(relatedNode);
			_log.append(":");
			_log.append(pME->attrName());
			_log.append(":");
			_log.append(pME->prevValue());
			_log.append(":");
			_log.append(pME->newValue());
		}
		_log.append("\n");
		
		if (_cancel) evt->stopPropagation();
		if (_readd)
			pCurrentTarget->addEventListener(type, Ptr(this, true), _capture);
	}
	
	static const XMLString& log()
	{
		return _log;
	}
	
	static void reset()
	{
		_log.clear();
	}

protected:
	~TestEventListener()
	{
	}

private:
	XMLString _name;
	bool      _cancel;
	bool      _readd;
	bool      _capture;
	static XMLString _log;
};


XMLString TestEventListener::_log;


EventTest::EventTest(const std::string& name): CppUnit::TestCase(name)
{
}


EventTest::~EventTest()
{
}


void EventTest::testInsert()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap");
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docListener, false);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docCapListener, true);
	
	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootListener, false);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootCapListener, true);
	
	pDoc->appendChild(pRoot);
	
	const XMLString& log = TestEventListener::log();

	assertTrue (log ==
		"docCap:DOMNodeInserted:CAPTURING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"rootCap:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"root:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"doc:DOMNodeInserted:BUBBLING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:root:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"root:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
	);
	
	TestEventListener::reset();
	
	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);

	assertTrue (log ==
		"docCap:DOMNodeInserted:CAPTURING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"rootCap:DOMNodeInserted:CAPTURING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"root:DOMNodeInserted:BUBBLING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"doc:DOMNodeInserted:BUBBLING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:CAPTURING_PHASE:root:#document:B:-:MODIFICATION::::\n"
		"rootCap:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"root:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:BUBBLING_PHASE:root:#document:B:-:MODIFICATION::::\n"
	);
}


void EventTest::testInsertSubtree()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap");
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docListener, false);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docCapListener, true);
	
	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootListener, false);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootCapListener, true);
	
	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);

	TestEventListener::reset();

	pDoc->appendChild(pRoot);

	const XMLString& log = TestEventListener::log();
	assertTrue (log ==
		"docCap:DOMNodeInserted:CAPTURING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"rootCap:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"root:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"doc:DOMNodeInserted:BUBBLING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:root:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"root:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
	);
}


void EventTest::testRemove()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap");
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeRemoved, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, docListener, false);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeRemoved, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, docCapListener, true);
	
	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeRemoved, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, rootListener, false);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeRemoved, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, rootCapListener, true);
	
	pDoc->appendChild(pRoot);
	
	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);

	TestEventListener::reset();

	pRoot->removeChild(pText);

	const XMLString& log = TestEventListener::log();
	assertTrue (log ==
		"docCap:DOMNodeRemoved:CAPTURING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"rootCap:DOMNodeRemoved:CAPTURING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"root:DOMNodeRemoved:BUBBLING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"doc:DOMNodeRemoved:BUBBLING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"docCap:DOMNodeRemovedFromDocument:CAPTURING_PHASE:#text:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeRemovedFromDocument:CAPTURING_PHASE:#text:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:CAPTURING_PHASE:root:#document:B:-:MODIFICATION::::\n"
		"rootCap:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"root:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:BUBBLING_PHASE:root:#document:B:-:MODIFICATION::::\n"
	);
}


void EventTest::testRemoveSubtree()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap");
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeRemoved, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, docListener, false);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeRemoved, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, docCapListener, true);
	
	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeRemoved, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, rootListener, false);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeRemoved, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeRemovedFromDocument, rootCapListener, true);
	
	pDoc->appendChild(pRoot);
	
	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);

	TestEventListener::reset();

	pDoc->removeChild(pRoot);

	const XMLString& log = TestEventListener::log();	
	assertTrue (log ==
		"docCap:DOMNodeRemoved:CAPTURING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"rootCap:DOMNodeRemoved:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"root:DOMNodeRemoved:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"doc:DOMNodeRemoved:BUBBLING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"docCap:DOMNodeRemovedFromDocument:CAPTURING_PHASE:root:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeRemovedFromDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"root:DOMNodeRemovedFromDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"docCap:DOMNodeRemovedFromDocument:CAPTURING_PHASE:#text:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeRemovedFromDocument:CAPTURING_PHASE:#text:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
	);
}


void EventTest::testCharacterData()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");
	RefPtr<Text> pText = pDoc->createTextNode("text");

	pRoot->appendChild(pText);
	pDoc->appendChild(pRoot);

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap");
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");
	TestEventListener::Ptr textListener = new TestEventListener("text");
	TestEventListener::Ptr textCapListener = new TestEventListener("textCap");

	pDoc->addEventListener(MutationEvent::DOMCharacterDataModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMCharacterDataModified, docCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMCharacterDataModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMCharacterDataModified, rootCapListener, true);
	pText->addEventListener(MutationEvent::DOMCharacterDataModified, textListener, false);
	pText->addEventListener(MutationEvent::DOMCharacterDataModified, textCapListener, true);

	TestEventListener::reset();

	pText->setData("modified");

	const XMLString& log = TestEventListener::log();
	assertTrue (log ==
		"docCap:DOMCharacterDataModified:CAPTURING_PHASE:#text:#document:B:-:MODIFICATION:::text:modified\n"
		"rootCap:DOMCharacterDataModified:CAPTURING_PHASE:#text:root:B:-:MODIFICATION:::text:modified\n"
		"textCap:DOMCharacterDataModified:AT_TARGET:#text:#text:B:-:MODIFICATION:::text:modified\n"
		"text:DOMCharacterDataModified:AT_TARGET:#text:#text:B:-:MODIFICATION:::text:modified\n"
		"root:DOMCharacterDataModified:BUBBLING_PHASE:#text:root:B:-:MODIFICATION:::text:modified\n"
		"doc:DOMCharacterDataModified:BUBBLING_PHASE:#text:#document:B:-:MODIFICATION:::text:modified\n"
	);
}


void EventTest::testCancel()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");
	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);
	pDoc->appendChild(pRoot);

	TestEventListener::Ptr docListener = new TestEventListener("doc");
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap", true);
	TestEventListener::Ptr rootListener = new TestEventListener("root");
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap");
	TestEventListener::Ptr textListener = new TestEventListener("text");
	TestEventListener::Ptr textCapListener = new TestEventListener("textCap");

	pDoc->addEventListener(MutationEvent::DOMCharacterDataModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMCharacterDataModified, docCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMCharacterDataModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMCharacterDataModified, rootCapListener, true);
	pText->addEventListener(MutationEvent::DOMCharacterDataModified, textListener, false);
	pText->addEventListener(MutationEvent::DOMCharacterDataModified, textCapListener, true);

	TestEventListener::reset();
	
	pText->setData("modified");

	const XMLString& log = TestEventListener::log();
	assertTrue (log == "docCap:DOMCharacterDataModified:CAPTURING_PHASE:#text:#document:B:-:MODIFICATION:::text:modified\n");
}


void EventTest::testAttributes()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr rootListener = new TestEventListener("root");
	pRoot->addEventListener(MutationEvent::DOMAttrModified, rootListener, false);
	
	pRoot->setAttribute("a1", "v1");

	const XMLString& log = TestEventListener::log();
	assertTrue (log == "root:DOMAttrModified:AT_TARGET:root:root:B:-:ADDITION:a1:a1::v1\n");
	
	TestEventListener::reset();
	pRoot->setAttribute("a1", "V1");
	assertTrue (log == "root:DOMAttrModified:AT_TARGET:root:root:B:-:MODIFICATION:a1:a1:v1:V1\n");
	
	TestEventListener::reset();
	pRoot->setAttribute("a2", "v2");
	assertTrue (log == "root:DOMAttrModified:AT_TARGET:root:root:B:-:ADDITION:a2:a2::v2\n");

	TestEventListener::reset();
	pRoot->removeAttribute("a1");	
	assertTrue (log == "root:DOMAttrModified:AT_TARGET:root:root:B:-:REMOVAL:a1:a1:V1:\n");
}


void EventTest::testAddRemoveInEvent()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	TestEventListener::Ptr docListener = new TestEventListener("doc", false, true, false);
	TestEventListener::Ptr docCapListener = new TestEventListener("docCap", false, true, true);
	TestEventListener::Ptr rootListener = new TestEventListener("root", false, true, false);
	TestEventListener::Ptr rootCapListener = new TestEventListener("rootCap", false, true, true);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docListener, false);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docListener, false);

	pDoc->addEventListener(MutationEvent::DOMSubtreeModified, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInserted, docCapListener, true);
	pDoc->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, docCapListener, true);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootListener, false);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootListener, false);

	pRoot->addEventListener(MutationEvent::DOMSubtreeModified, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInserted, rootCapListener, true);
	pRoot->addEventListener(MutationEvent::DOMNodeInsertedIntoDocument, rootCapListener, true);

	pDoc->appendChild(pRoot);//!!!

	const XMLString& log = TestEventListener::log();
	assertTrue (log ==
		"docCap:DOMNodeInserted:CAPTURING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"rootCap:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"root:DOMNodeInserted:AT_TARGET:root:root:B:-:MODIFICATION:#document:::\n"
		"doc:DOMNodeInserted:BUBBLING_PHASE:root:#document:B:-:MODIFICATION:#document:::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:root:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"root:DOMNodeInsertedIntoDocument:AT_TARGET:root:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:AT_TARGET:#document:#document:B:-:MODIFICATION::::\n"
	);
	
	TestEventListener::reset();

	RefPtr<Text> pText = pDoc->createTextNode("text");
	pRoot->appendChild(pText);

	assertTrue (log ==
		"docCap:DOMNodeInserted:CAPTURING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"rootCap:DOMNodeInserted:CAPTURING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"root:DOMNodeInserted:BUBBLING_PHASE:#text:root:B:-:MODIFICATION:root:::\n"
		"doc:DOMNodeInserted:BUBBLING_PHASE:#text:#document:B:-:MODIFICATION:root:::\n"
		"docCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:#document:-:-:MODIFICATION::::\n"
		"rootCap:DOMNodeInsertedIntoDocument:CAPTURING_PHASE:#text:root:-:-:MODIFICATION::::\n"
		"docCap:DOMSubtreeModified:CAPTURING_PHASE:root:#document:B:-:MODIFICATION::::\n"
		"rootCap:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"root:DOMSubtreeModified:AT_TARGET:root:root:B:-:MODIFICATION::::\n"
		"doc:DOMSubtreeModified:BUBBLING_PHASE:root:#document:B:-:MODIFICATION::::\n"
	);
}


void EventTest::testSuspended()
{
	RefPtr<Document> pDoc = new Document;
	RefPtr<Element> pRoot = pDoc->createElement("root");

	pDoc->suspendEvents();

	TestEventListener::Ptr rootListener = new TestEventListener("root");
	pRoot->addEventListener(MutationEvent::DOMAttrModified, rootListener, false);

	pRoot->setAttribute("a1", "v1");

	const XMLString& log = TestEventListener::log();
	assertTrue (log.empty());

	TestEventListener::reset();
	pRoot->setAttribute("a1", "V1");
	assertTrue (log.empty());

	TestEventListener::reset();
	pRoot->setAttribute("a2", "v2");
	assertTrue (log.empty());

	TestEventListener::reset();
	pRoot->removeAttribute("a1");
	assertTrue (log.empty());
}


void EventTest::setUp()
{
	TestEventListener::reset();
}


void EventTest::tearDown()
{
}


CppUnit::Test* EventTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("EventTest");

	CppUnit_addTest(pSuite, EventTest, testInsert);
	CppUnit_addTest(pSuite, EventTest, testInsertSubtree);
	CppUnit_addTest(pSuite, EventTest, testRemove);
	CppUnit_addTest(pSuite, EventTest, testRemoveSubtree);
	CppUnit_addTest(pSuite, EventTest, testCharacterData);
	CppUnit_addTest(pSuite, EventTest, testCancel);
	CppUnit_addTest(pSuite, EventTest, testAttributes);
	CppUnit_addTest(pSuite, EventTest, testAddRemoveInEvent);
	CppUnit_addTest(pSuite, EventTest, testSuspended);

	return pSuite;
}
