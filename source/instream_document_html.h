#pragma once

#include "instream_document_trec.h"


namespace JASS
{
  class instream_document_html : public instream_document_trec 
  {
  public :
    instream_document_html() = delete;
    
  instream_document_html(std::shared_ptr<instream> &source, const std::string &document_tag = "html", const std::string &document_primary_key_tag = "title") : instream_document_trec(source, document_tag, document_primary_key_tag)
      {
	// nothing
      }

    virtual ~instream_document_html()
      {
	// nothing
      }

    virtual void set_tags(const std::string &document_tag, const std::string &primary_key_tag)
    {
      instream_document_trec::set_tags(document_tag, primary_key_tag);
      document_start_tag = "<" + document_tag;
    }
  } ;

}
