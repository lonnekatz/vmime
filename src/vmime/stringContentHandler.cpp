//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/stringContentHandler.hpp"

#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/streamUtils.hpp"


namespace vmime {


stringContentHandler::stringContentHandler() {

}


stringContentHandler::stringContentHandler(
	const string& buffer,
	const vmime::encoding& enc
)
	: m_encoding(enc),
	  m_string(buffer) {

}


stringContentHandler::stringContentHandler(
	const stringContentHandler& cts
)
	: contentHandler(),
	  m_contentType(cts.m_contentType),
	  m_encoding(cts.m_encoding),
	  m_string(cts.m_string) {

}


stringContentHandler::~stringContentHandler() {

}


shared_ptr <contentHandler> stringContentHandler::clone() const {

	return make_shared <stringContentHandler>(*this);
}


stringContentHandler& stringContentHandler::operator=(const stringContentHandler& cts) {

	m_contentType = cts.m_contentType;
	m_encoding = cts.m_encoding;
	m_string = cts.m_string;

	return *this;
}


void stringContentHandler::setData(const string& buffer, const vmime::encoding& enc) {

	m_encoding = enc;
	m_string = buffer;
}


stringContentHandler& stringContentHandler::operator=(const string& buffer) {

	setData(buffer, NO_ENCODING);
	return *this;
}


void stringContentHandler::generate(
	utility::outputStream& os,
	const vmime::encoding& enc,
	const size_t maxLineLength
) const {

	// Managed data is already encoded
	if (isEncoded()) {

		// The data is already encoded but the encoding specified for
		// the generation is different from the current one. We need
		// to re-encode data: decode from input buffer to temporary
		// buffer, and then re-encode to output stream...
		if (m_encoding != enc) {

			shared_ptr <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();
			shared_ptr <utility::encoder::encoder> theEncoder = enc.getEncoder();

			theEncoder->getProperties()["maxlinelength"] = maxLineLength;
			theEncoder->getProperties()["text"] = (m_contentType.getType() == mediaTypes::TEXT);

			utility::inputStreamStringAdapter in(m_string);

			std::ostringstream oss;
			utility::outputStreamAdapter tempOut(oss);

			theDecoder->decode(in, tempOut);

			string str = oss.str();
			utility::inputStreamStringAdapter tempIn(str);

			theEncoder->encode(tempIn, os);

		// No encoding to perform
		} else {

			os.write(m_string.data(), m_string.length());
		}

	// Need to encode data before
	} else {

		shared_ptr <utility::encoder::encoder> theEncoder = enc.getEncoder();
		theEncoder->getProperties()["maxlinelength"] = maxLineLength;
		theEncoder->getProperties()["text"] = (m_contentType.getType() == mediaTypes::TEXT);

		utility::inputStreamStringAdapter in(m_string);

		theEncoder->encode(in, os);
	}
}


void stringContentHandler::extract(
	utility::outputStream& os,
	utility::progressListener* progress
) const {

	// No decoding to perform
	if (!isEncoded()) {

		utility::inputStreamStringAdapter in(m_string);
		utility::progressListenerSizeAdapter plsa(progress, getLength());

		utility::bufferedStreamCopy(in, os, m_string.length(), progress);

	// Need to decode data
	} else {

		shared_ptr <utility::encoder::encoder> theDecoder = m_encoding.getEncoder();

		utility::inputStreamStringAdapter in(m_string);
		utility::progressListenerSizeAdapter plsa(progress, getLength());

		theDecoder->decode(in, os, &plsa);
	}
}


void stringContentHandler::extractRaw(
	utility::outputStream& os,
	utility::progressListener* progress
) const {

	utility::inputStreamStringAdapter in(m_string);
	utility::progressListenerSizeAdapter plsa(progress, getLength());

	utility::bufferedStreamCopy(in, os, m_string.length(), progress);
}


size_t stringContentHandler::getLength() const {

	return m_string.length();
}


bool stringContentHandler::isEmpty() const {

	return m_string.length() == 0;
}


bool stringContentHandler::isEncoded() const {

	return m_encoding != NO_ENCODING;
}


const vmime::encoding& stringContentHandler::getEncoding() const {

	return m_encoding;
}


bool stringContentHandler::isBuffered() const {

	return true;
}


void stringContentHandler::setContentTypeHint(const mediaType& type) {

	m_contentType = type;
}


const mediaType stringContentHandler::getContentTypeHint() const {

	return m_contentType;
}


} // vmime
