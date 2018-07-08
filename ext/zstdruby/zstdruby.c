#include "zstdruby.h"
#include "./libzstd/zstd.h"

VALUE rb_mZstd;
VALUE cDecompressionStream;

static VALUE zstdVersion(VALUE self)
{
  unsigned version = ZSTD_versionNumber();
  return INT2NUM(version);
}

static VALUE compress(int argc, VALUE *argv, VALUE self)
{
  VALUE input_value;
  VALUE compression_level_value;
  rb_scan_args(argc, argv, "11", &input_value, &compression_level_value);

  Check_Type(input_value, RUBY_T_STRING);
  const char* input_data = RSTRING_PTR(input_value);
  size_t input_size = RSTRING_LEN(input_value);

  int compression_level;
  if (NIL_P(compression_level_value)) {
    compression_level = 0; // The default. See ZSTD_CLEVEL_DEFAULT in zstd_compress.c
  } else {
    compression_level = NUM2INT(compression_level_value);
  }

  // do compress
  size_t max_compressed_size = ZSTD_compressBound(input_size);

  VALUE output = rb_str_new(NULL, max_compressed_size);
  char* output_data = RSTRING_PTR(output);

  size_t compressed_size = ZSTD_compress((void*)output_data, max_compressed_size,
                                         (const void*)input_data, input_size, compression_level);

  if (ZSTD_isError(compressed_size)) {
    rb_raise(rb_eRuntimeError, "%s: %s", "compress failed", ZSTD_getErrorName(compressed_size));
  } else {
    rb_str_resize(output, compressed_size);
  }

  return output;
}

VALUE rescue_nil(VALUE obj1)
{
  /* handle exception */
  return Qnil;
}

VALUE enumerator_next(VALUE enumerator)
{
  return rb_funcall(enumerator, rb_intern("next"), 0);
}

VALUE next(VALUE enumerator)
{
  return rb_rescue2(enumerator_next, enumerator, rescue_nil, Qnil, rb_eStopIteration, 0);
}

static ZSTD_DStream* create_dstream() {
  ZSTD_DStream* dstream = ZSTD_createDStream();
  if (dstream == NULL) {
    rb_raise(rb_eRuntimeError, "%s", "ZSTD_createDStream failed");
  }

  size_t initResult = ZSTD_initDStream(dstream);
  if (ZSTD_isError(initResult)) {
    ZSTD_freeDStream(dstream);
    rb_raise(rb_eRuntimeError, "%s: %s", "ZSTD_initDStream failed", ZSTD_getErrorName(initResult));
  }
  return dstream;
}

struct DecompressionStream
{
  ZSTD_DStream* dstream;
  ZSTD_outBuffer* output;
  ZSTD_inBuffer* input;
  // VALUE enumerator;
};

void zstd_decompression_stream_free(void* data)
{
  rb_eval_string("p 'zstd free'");
  // struct DecompressionStream* decompression_stream = (struct DecompressionStream*)data;
  // if (decompression_stream->input->src != NULL) {
  //   rb_eval_string("p 'src'");
  //   VALUE str = rb_str_new(decompression_stream->input->src, strlen(decompression_stream->input->src));
  //   rb_p(str);
  //   free(&decompression_stream->input->src);
  //   decompression_stream->input->src = NULL;
  // }
  // rb_eval_string("p 'done str free'");
  // if (data != NULL) {
  //   free(data);
  //   data = NULL;
  // }
  // rb_eval_string("p 'done free'");
}


void zstd_decompression_stream_mark(void* data)
{
  // rb_gc_mark(((struct DecompressionStream*)data)->enumerator);
}


size_t zstd_decompression_stream_size(const void* data)
{
  return sizeof(struct DecompressionStream);
}

static const rb_data_type_t zstd_decompression_stream_type = {
  .wrap_struct_name = "zstd_decompression_stream_type",
  .function = {
    .dmark = zstd_decompression_stream_mark,
    .dfree = zstd_decompression_stream_free,
    .dsize = zstd_decompression_stream_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE zstd_decompression_stream_alloc(VALUE self)
{
  struct DecompressionStream* data = malloc(sizeof(struct DecompressionStream));
  rb_eval_string("p 'alloc'");
  return TypedData_Wrap_Struct(self, &zstd_decompression_stream_type, data);
}

// VALUE zstd_decompression_stream_m_initialize(VALUE self, VALUE enumerator)
// {
//   struct DecompressionStream* data;
//   TypedData_Get_Struct(self, struct DecompressionStream, &zstd_decompression_stream_type, data);
//   data->dstream = create_dstream();
//   ZSTD_inBuffer input = { NULL, 0, 0};
//   data->input = &input;
//   ZSTD_outBuffer output = { RSTRING_PTR(rb_str_new(NULL, ZSTD_DStreamOutSize())), ZSTD_DStreamOutSize(), 0 };
//   data->output = &output;
//   data->enumerator = enumerator;
//   rb_p(LONG2FIX(&data->input->src));
//   rb_p(LONG2FIX(data->input->src));
//   rb_p(LONG2FIX(NULL));
//   rb_eval_string("p 'init'");
//   return self;
// }


static VALUE decompression_stream_free(VALUE decompression_stream) {
  rb_eval_string("p 'freeingaaa'");
  // struct DecompressionStream* decompression_stream_ptr;
  // TypedData_Get_Struct(
  //   decompression_stream,
  //   struct DecompressionStream,
  //   &zstd_decompression_stream_type,
  //   decompression_stream_ptr
  // );
  // rb_p(LONG2FIX(decompression_stream_ptr->input->size));
  // rb_p(LONG2FIX(decompression_stream_ptr->input->pos));
  // if (decompression_stream_ptr->input->src != NULL) {
  //   rb_eval_string("p 'src'");
  //   free(&decompression_stream_ptr->input->src);
  //   decompression_stream_ptr->input->src = NULL;
  // }
  // rb_p(LONG2FIX(decompression_stream_ptr->output->dst));
  // rb_p(LONG2FIX(decompression_stream_ptr->output->dst));
  // // if (decompression_stream_ptr->output->dst != NULL) {
  // //   rb_eval_string("p 'dst'");
  // //   free(&decompression_stream_ptr->output->dst);
  // //   decompression_stream_ptr->output->dst = NULL;
  // // }
  // rb_eval_string("p 'done str free'");
  // if (decompression_stream_ptr != NULL) {
  //   free(decompression_stream_ptr);
  //   decompression_stream_ptr = NULL;
  // }
  // rb_p(decompression_stream);
  return Qnil;
}

static VALUE decompress_stream(VALUE decompression_stream, VALUE enumerator) {
  rb_eval_string("p 'decompression_stream'");
  struct DecompressionStream* decompression_stream_ptr;
  TypedData_Get_Struct(
    decompression_stream,
    struct DecompressionStream,
    &zstd_decompression_stream_type,
    decompression_stream_ptr
  );
  VALUE buffer = Qnil;
  while (Qnil != (buffer = next(enumerator))) {
    rb_eval_string("p 'loop'");
    Check_Type(buffer, T_STRING);
    const char* input_data = RSTRING_PTR(buffer);
    size_t input_size = RSTRING_LEN(buffer);
    ZSTD_inBuffer input = { input_data, input_size, 0 };
    free(&decompression_stream_ptr->input->src);
    free(&decompression_stream_ptr->input);
    decompression_stream_ptr->input = &input;
    size_t readHint;
    while (input.pos < input.size) {
      readHint = ZSTD_decompressStream(
        decompression_stream_ptr->dstream,
        decompression_stream_ptr->output,
        decompression_stream_ptr->input
      );
      if (ZSTD_isError(readHint)) {
        rb_raise(rb_eRuntimeError, "%s: %s", "ZSTD_decompressStream failed", ZSTD_getErrorName(readHint));
      }
      if (decompression_stream_ptr->output->pos > 0) {
        VALUE output_buffer = rb_str_new(decompression_stream_ptr->output->dst, strlen(decompression_stream_ptr->output->dst));
        rb_str_resize(
          output_buffer,
          decompression_stream_ptr->output->pos
        );
        int state;
        VALUE res = rb_protect(rb_yield, output_buffer, &state);
        if (state) {
          decompression_stream_free(decompression_stream);
          rb_jump_tag(state);
        }
        decompression_stream_ptr->output->dst = RSTRING_PTR(rb_str_new(NULL, ZSTD_DStreamOutSize()));
        decompression_stream_ptr->output->pos = 0;
      }
    }
    if (readHint == 0) {
      // Handle concatenated streams
      decompress_stream(decompression_stream, enumerator);
    }
  }
  rb_eval_string("p 'fin'");
  return Qnil;
}




struct FooS
{
  ZSTD_DStream* dstream;
  ZSTD_outBuffer* output;
  ZSTD_inBuffer* input;
  // VALUE enumerator;
};

void foo_free(void* data)
{
  free(data);
}

size_t foo_size(const void* data)
{
  return sizeof(struct FooS);
}

static const rb_data_type_t foo_type = {
  .wrap_struct_name = "foo",
  .function = {
    .dmark = NULL,
    .dfree = foo_free,
    .dsize = foo_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE foo_alloc(VALUE self)
{
  /* allocate */
  int* data = malloc(sizeof(struct FooS));

  /* wrap */
  return TypedData_Wrap_Struct(self, &foo_type, data);
}

VALUE foo_m_initialize(VALUE self, VALUE val)
{
  struct FooS * foo;
  TypedData_Get_Struct(self, struct FooS, &foo_type, foo);


  foo->dstream = create_dstream();
  ZSTD_inBuffer input = { NULL, 0, 0};
  foo->input = &input;
  ZSTD_outBuffer output = { RSTRING_PTR(rb_str_new(NULL, ZSTD_DStreamOutSize())), ZSTD_DStreamOutSize(), 0 };
  foo->output = &output;
  // *data = NUM2INT(val);

  return self;
}


static VALUE decompress_streaming(VALUE self, VALUE enumerator)
{
  if (rb_block_given_p()) {
    VALUE cFoo = rb_define_class("Foo", rb_cData);
    rb_define_alloc_func(cFoo, foo_alloc);
    rb_define_method(cFoo, "initialize", foo_m_initialize, 0);

    VALUE obj = rb_eval_string("Foo.new()");
    rb_p(obj);
    // struct DecompressionStream* decompression_stream_ptr;
    // VALUE decompression_stream = TypedData_Make_Struct(
    //   cDecompressionStream,
    //   struct DecompressionStream,
    //   &zstd_decompression_stream_type,
    //   decompression_stream_ptr
    // );
    // decompression_stream_ptr->dstream = create_dstream();
    // ZSTD_inBuffer input = { NULL, 0, 0};
    // decompression_stream_ptr->input = &input;
    // ZSTD_outBuffer output = { RSTRING_PTR(rb_str_new(NULL, ZSTD_DStreamOutSize())), ZSTD_DStreamOutSize(), 0 };
    // decompression_stream_ptr->output = &output;
    return Qnil;
    // return decompress_stream(decompression_stream);
    // return rb_ensure(decompress_stream, decompression_stream, decompression_stream_free, decompression_stream);
  } else {
    return rb_funcall(self, rb_intern("to_enum"), 2, ID2SYM(rb_intern("decompress_streaming")), enumerator);
  }
}

static VALUE decompress_buffered(const char* input_data, size_t input_size)
{
  const size_t outputBufferSize = 4096;

  ZSTD_DStream* const dstream = create_dstream();

  VALUE output_string = rb_str_new(NULL, 0);
  ZSTD_outBuffer output = { NULL, 0, 0 };

  ZSTD_inBuffer input = { input_data, input_size, 0 };
  while (input.pos < input.size) {
    output.size += outputBufferSize;
    rb_str_resize(output_string, output.size);
    output.dst = RSTRING_PTR(output_string);

    size_t readHint = ZSTD_decompressStream(dstream, &output, &input);
    if (ZSTD_isError(readHint)) {
      ZSTD_freeDStream(dstream);
      rb_raise(rb_eRuntimeError, "%s: %s", "ZSTD_decompressStream failed", ZSTD_getErrorName(readHint));
    }
  }

  ZSTD_freeDStream(dstream);
  rb_str_resize(output_string, output.pos);
  return output_string;
}

static VALUE decompress(VALUE self, VALUE input)
{
  Check_Type(input, T_STRING);
  const char* input_data = RSTRING_PTR(input);
  size_t input_size = RSTRING_LEN(input);

  uint64_t uncompressed_size = ZSTD_getDecompressedSize(input_data, input_size);

  if (uncompressed_size == 0) {
    return decompress_buffered(input_data, input_size);
  }

  VALUE output = rb_str_new(NULL, uncompressed_size);
  char* output_data = RSTRING_PTR(output);

  size_t decompress_size = ZSTD_decompress((void*)output_data, uncompressed_size,
                                           (const void*)input_data, input_size);

  if (ZSTD_isError(decompress_size)) {
    rb_raise(rb_eRuntimeError, "%s: %s", "decompress error", ZSTD_getErrorName(decompress_size));
  }

  return output;
}

void
Init_zstdruby(void)
{
  rb_mZstd = rb_define_module("Zstd");
  rb_define_module_function(rb_mZstd, "zstd_version", zstdVersion, 0);
  rb_define_module_function(rb_mZstd, "compress", compress, -1);
  rb_define_module_function(rb_mZstd, "decompress", decompress, 1);
  rb_define_module_function(rb_mZstd, "decompress_streaming", decompress_streaming, 1);
  cDecompressionStream = rb_define_class_under(rb_mZstd, "DecompressionStream", rb_cData);
  // rb_define_alloc_func(cDecompressionStream, zstd_decompression_stream_alloc);
  // rb_define_method(cDecompressionStream, "initialize", zstd_decompression_stream_m_initialize, 1);
}
