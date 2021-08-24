/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2021 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DP_NET_ENVELOPEBUILDER_H
#define DP_NET_ENVELOPEBUILDER_H

#include <cstdint>

class QImage;

namespace rustpile {
	struct MessageWriter;
}

namespace net {

class Envelope;

/**
 * @brief A helper class for using Rustpile MessageWriter to create message envelopes
 */
class EnvelopeBuilder {
public:
	EnvelopeBuilder();
	~EnvelopeBuilder();

	EnvelopeBuilder(const EnvelopeBuilder&) = delete;
	EnvelopeBuilder &operator=(const EnvelopeBuilder&) = delete;

	Envelope toEnvelope();

	operator rustpile::MessageWriter*() const { return m_writer; }

	/// Helper function: write a PutImage command using QImage
	void buildPutQImage(uint8_t ctxid, uint16_t layer, int x, int y, const QImage &image, uint8_t mode);

private:
	rustpile::MessageWriter *m_writer;
};
}

#endif