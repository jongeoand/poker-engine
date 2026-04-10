#include "range/iterate.h"

void combostream_init(HtrComboStream* s, const HandTypeRange* htr, uint64_t dead) {
	s->htr = htr;
	s->dead = dead;
	s->ht_index = 0;
	s->buf_n = 0;
	s->buf_i = 0;
}

bool combostream_next(HtrComboStream* stream, Combo* out) {
	if (stream->buf_i < stream->buf_n) {
		*out = stream->buf[stream->buf_i++];
		return true;
	}

	while (stream->ht_index < HANDTYPE_COUNT) {
		HandType ht = handtype_from_index(stream->ht_index++);
		if (!htr_contains(stream->htr, ht)) continue;

		stream->buf_n = handtype_combos(ht, stream->dead, stream->buf);
		stream->buf_i = 0;

		if (stream->buf_n == 0) continue;

		*out = stream->buf[stream->buf_i++];
		return true;
	}

	return false;
}

void stream_foreach_combo(const HandTypeRange* htr, uint64_t dead, ComboFn fn, void* ctx) {
	HtrComboStream stream;
	Combo c;

	combostream_init(&stream, htr, dead);

	while (combostream_next(&stream, &c)) {
		fn(c, ctx);
	}
}
