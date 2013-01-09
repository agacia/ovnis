/**
 *
 *
 * Copyright (c) 2010-2011 University of Luxembourg
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * @file XMLSumoConfParser.cpp
 * @date Apr 19, 2010
 *
 * @author Yoann Pigné <yoann@pigne.org>
 */
#include "xml-sumo-conf-parser.h"
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

//#include<stl_vector.h>
using namespace std;

using namespace xercesc;

XMLSumoConfParser::XMLSumoConfParser()
{
	is_net_file_name = false;
	is_location = false;
	is_port=false;
}

void
XMLSumoConfParser::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs)
{
	char* message = XMLString::transcode(localname);
	string name(message);
	if ("net-file" == name)
	{
		is_net_file_name = true;
		XMLCh* q = XMLString::transcode("value");
		char* b = XMLString::transcode(attrs.getValue(q));
		net_file_name = string(b);
		XMLString::release(&b);
		XMLString::release(&q);
	}

	if("remote-port" == name)
	{
		is_port=true;
		XMLCh* q = XMLString::transcode("value");
		char* b = XMLString::transcode(attrs.getValue(q));
		(*port)=atoi(b);
		XMLString::release(&b);
		XMLString::release(&q);
	}
	if ("location" == name)
	{
		XMLCh* q = XMLString::transcode("convBoundary");
		char* b = XMLString::transcode(attrs.getValue(q));

		char * pch;
		pch = strtok(b, ",");
		pch = strtok(NULL, ",");
		boundaries[0] = atof(strtok(NULL, ","));
		boundaries[1] = atof(strtok(NULL, ","));

		XMLString::release(&b);
		XMLString::release(&q);
	}
	XMLString::release(&message);
}

void
XMLSumoConfParser::fatalError(const SAXParseException& exception)
{
  char* message = XMLString::transcode(exception.getMessage());
  cerr << "Fatal Error: " << message << " at line: " << exception.getLineNumber() << endl;
  XMLString::release(&message);
}

void
XMLSumoConfParser::characters(const XMLCh* const xMLCh, const unsigned int xMLSize_t)
{

  if (is_net_file_name)
  {
    is_net_file_name = false;

  }
  else
  {
    if (is_port)
    {
		is_port = false;
    }
  }

}

void
XMLSumoConfParser::parseConfiguration(const string & filename, int * p, double* bound)
{

  string base = filename.substr(0, filename.find_last_of('/'));
  //cerr << "basename:" << base << endl;
  try
  {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch)
  {
    char* message = XMLString::transcode(toCatch.getMessage());
    cerr << "Error during initialization! :\n";
    cerr << "Exception message is: \n" << message << "\n";
    XMLString::release(&message);
    return;
  }

  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
  parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
  parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true); // optional

  XMLSumoConfParser* defaultHandler = new XMLSumoConfParser();
  defaultHandler->port = p;
  defaultHandler->boundaries = bound;
  parser->setContentHandler(defaultHandler);
  parser->setErrorHandler(defaultHandler);

  try
  {
    parser->parse(filename.c_str());

    parser->parse((base + "/" + defaultHandler->net_file_name).c_str());

  }
  catch (const XMLException& toCatch)
  {
    char* message = XMLString::transcode(toCatch.getMessage());
    cerr << "Exception message is: \n" << message << "\n";
    XMLString::release(&message);
    return;
  }
  catch (const SAXParseException& toCatch)
  {
    char* message = XMLString::transcode(toCatch.getMessage());
    cerr << "Exception message is: \n" << message << "\n";
    XMLString::release(&message);
    return;
  }
  catch (...)
  {
    cerr << "Unexpected Exception \n";
    return;
  }

  delete parser;
  delete defaultHandler;
}
