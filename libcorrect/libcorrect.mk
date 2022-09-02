# for libcorrect inclusion
LIBCORRECTROOT = $(VARIOUS)/../../../libcorrect
LIBCORRECTINC = $(LIBCORRECTROOT)/include
LIBCORRECTSRCDIR = $(LIBCORRECTROOT)/src

LIBCORRECTSRC = $(LIBCORRECTSRCDIR)/convolutional/bit.c \
	 $(LIBCORRECTSRCDIR)/reed-solomon/decode.c \
	 $(LIBCORRECTSRCDIR)/reed-solomon/encode.c \
	 $(LIBCORRECTSRCDIR)/reed-solomon/polynomial.c \
	 $(LIBCORRECTSRCDIR)/reed-solomon/reed-solomon.c \
	 $(LIBCORRECTSRCDIR)/convolutional/convolutional.c \
	 $(LIBCORRECTSRCDIR)/convolutional/cdecode.c \
	 $(LIBCORRECTSRCDIR)/convolutional/cencode.c \
	 $(LIBCORRECTSRCDIR)/convolutional/error_buffer.c \
	 $(LIBCORRECTSRCDIR)/convolutional/history_buffer.c \
	 $(LIBCORRECTSRCDIR)/convolutional/lookup.c \
	 $(LIBCORRECTSRCDIR)/convolutional/metric.c
