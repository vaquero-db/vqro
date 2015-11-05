// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: core.proto

#ifndef PROTOBUF_core_2eproto__INCLUDED
#define PROTOBUF_core_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/map.h>
#include <google/protobuf/map_field_inl.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace vqro {
namespace rpc {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_core_2eproto();
void protobuf_AssignDesc_core_2eproto();
void protobuf_ShutdownFile_core_2eproto();

class Datapoint;
class Series;
class StatusMessage;

// ===================================================================

class Series : public ::google::protobuf::Message {
 public:
  Series();
  virtual ~Series();

  Series(const Series& from);

  inline Series& operator=(const Series& from) {
    CopyFrom(from);
    return *this;
  }

  inline ::google::protobuf::Arena* GetArena() const { return GetArenaNoVirtual(); }
  inline void* GetMaybeArenaPointer() const {
    return MaybeArenaPtr();
  }
  static const ::google::protobuf::Descriptor* descriptor();
  static const Series& default_instance();

  void UnsafeArenaSwap(Series* other);
  void Swap(Series* other);

  // implements Message ----------------------------------------------

  inline Series* New() const { return New(NULL); }

  Series* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Series& from);
  void MergeFrom(const Series& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(Series* other);
  protected:
  explicit Series(::google::protobuf::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::google::protobuf::Arena* arena);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------


  // accessors -------------------------------------------------------

  // map<string, string> labels = 1;
  int labels_size() const;
  void clear_labels();
  static const int kLabelsFieldNumber = 1;
  const ::google::protobuf::Map< ::std::string, ::std::string >&
      labels() const;
  ::google::protobuf::Map< ::std::string, ::std::string >*
      mutable_labels();

  // @@protoc_insertion_point(class_scope:vqro.rpc.Series)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  friend class ::google::protobuf::Arena;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  bool _is_default_instance_;
  typedef ::google::protobuf::internal::MapEntryLite<
      ::std::string, ::std::string,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      0 >
      Series_LabelsEntry;
  ::google::protobuf::internal::MapField<
      ::std::string, ::std::string,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      0 > labels_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_core_2eproto();
  friend void protobuf_AssignDesc_core_2eproto();
  friend void protobuf_ShutdownFile_core_2eproto();

  void InitAsDefaultInstance();
  static Series* default_instance_;
};
// -------------------------------------------------------------------

class Datapoint : public ::google::protobuf::Message {
 public:
  Datapoint();
  virtual ~Datapoint();

  Datapoint(const Datapoint& from);

  inline Datapoint& operator=(const Datapoint& from) {
    CopyFrom(from);
    return *this;
  }

  inline ::google::protobuf::Arena* GetArena() const { return GetArenaNoVirtual(); }
  inline void* GetMaybeArenaPointer() const {
    return MaybeArenaPtr();
  }
  static const ::google::protobuf::Descriptor* descriptor();
  static const Datapoint& default_instance();

  void UnsafeArenaSwap(Datapoint* other);
  void Swap(Datapoint* other);

  // implements Message ----------------------------------------------

  inline Datapoint* New() const { return New(NULL); }

  Datapoint* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Datapoint& from);
  void MergeFrom(const Datapoint& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(Datapoint* other);
  protected:
  explicit Datapoint(::google::protobuf::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::google::protobuf::Arena* arena);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int64 timestamp = 1;
  void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  ::google::protobuf::int64 timestamp() const;
  void set_timestamp(::google::protobuf::int64 value);

  // optional int64 duration = 2;
  void clear_duration();
  static const int kDurationFieldNumber = 2;
  ::google::protobuf::int64 duration() const;
  void set_duration(::google::protobuf::int64 value);

  // optional double value = 3;
  void clear_value();
  static const int kValueFieldNumber = 3;
  double value() const;
  void set_value(double value);

  // @@protoc_insertion_point(class_scope:vqro.rpc.Datapoint)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  friend class ::google::protobuf::Arena;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  bool _is_default_instance_;
  ::google::protobuf::int64 timestamp_;
  ::google::protobuf::int64 duration_;
  double value_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_core_2eproto();
  friend void protobuf_AssignDesc_core_2eproto();
  friend void protobuf_ShutdownFile_core_2eproto();

  void InitAsDefaultInstance();
  static Datapoint* default_instance_;
};
// -------------------------------------------------------------------

class StatusMessage : public ::google::protobuf::Message {
 public:
  StatusMessage();
  virtual ~StatusMessage();

  StatusMessage(const StatusMessage& from);

  inline StatusMessage& operator=(const StatusMessage& from) {
    CopyFrom(from);
    return *this;
  }

  inline ::google::protobuf::Arena* GetArena() const { return GetArenaNoVirtual(); }
  inline void* GetMaybeArenaPointer() const {
    return MaybeArenaPtr();
  }
  static const ::google::protobuf::Descriptor* descriptor();
  static const StatusMessage& default_instance();

  void UnsafeArenaSwap(StatusMessage* other);
  void Swap(StatusMessage* other);

  // implements Message ----------------------------------------------

  inline StatusMessage* New() const { return New(NULL); }

  StatusMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const StatusMessage& from);
  void MergeFrom(const StatusMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(StatusMessage* other);
  protected:
  explicit StatusMessage(::google::protobuf::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::google::protobuf::Arena* arena);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string text = 1;
  void clear_text();
  static const int kTextFieldNumber = 1;
  const ::std::string& text() const;
  void set_text(const ::std::string& value);
  void set_text(const char* value);
  void set_text(const char* value, size_t size);
  ::std::string* mutable_text();
  ::std::string* release_text();
  void set_allocated_text(::std::string* text);
  ::std::string* unsafe_arena_release_text();
  void unsafe_arena_set_allocated_text(
      ::std::string* text);

  // optional bool error = 2;
  void clear_error();
  static const int kErrorFieldNumber = 2;
  bool error() const;
  void set_error(bool value);

  // optional bool go_away = 3;
  void clear_go_away();
  static const int kGoAwayFieldNumber = 3;
  bool go_away() const;
  void set_go_away(bool value);

  // @@protoc_insertion_point(class_scope:vqro.rpc.StatusMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  friend class ::google::protobuf::Arena;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr text_;
  bool error_;
  bool go_away_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_core_2eproto();
  friend void protobuf_AssignDesc_core_2eproto();
  friend void protobuf_ShutdownFile_core_2eproto();

  void InitAsDefaultInstance();
  static StatusMessage* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Series

// map<string, string> labels = 1;
inline int Series::labels_size() const {
  return labels_.size();
}
inline void Series::clear_labels() {
  labels_.Clear();
}
inline const ::google::protobuf::Map< ::std::string, ::std::string >&
Series::labels() const {
  // @@protoc_insertion_point(field_map:vqro.rpc.Series.labels)
  return labels_.GetMap();
}
inline ::google::protobuf::Map< ::std::string, ::std::string >*
Series::mutable_labels() {
  // @@protoc_insertion_point(field_mutable_map:vqro.rpc.Series.labels)
  return labels_.MutableMap();
}

// -------------------------------------------------------------------

// Datapoint

// optional int64 timestamp = 1;
inline void Datapoint::clear_timestamp() {
  timestamp_ = GOOGLE_LONGLONG(0);
}
inline ::google::protobuf::int64 Datapoint::timestamp() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.Datapoint.timestamp)
  return timestamp_;
}
inline void Datapoint::set_timestamp(::google::protobuf::int64 value) {
  
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:vqro.rpc.Datapoint.timestamp)
}

// optional int64 duration = 2;
inline void Datapoint::clear_duration() {
  duration_ = GOOGLE_LONGLONG(0);
}
inline ::google::protobuf::int64 Datapoint::duration() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.Datapoint.duration)
  return duration_;
}
inline void Datapoint::set_duration(::google::protobuf::int64 value) {
  
  duration_ = value;
  // @@protoc_insertion_point(field_set:vqro.rpc.Datapoint.duration)
}

// optional double value = 3;
inline void Datapoint::clear_value() {
  value_ = 0;
}
inline double Datapoint::value() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.Datapoint.value)
  return value_;
}
inline void Datapoint::set_value(double value) {
  
  value_ = value;
  // @@protoc_insertion_point(field_set:vqro.rpc.Datapoint.value)
}

// -------------------------------------------------------------------

// StatusMessage

// optional string text = 1;
inline void StatusMessage::clear_text() {
  text_.ClearToEmpty(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArenaNoVirtual());
}
inline const ::std::string& StatusMessage::text() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.StatusMessage.text)
  return text_.Get(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void StatusMessage::set_text(const ::std::string& value) {
  
  text_.Set(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value, GetArenaNoVirtual());
  // @@protoc_insertion_point(field_set:vqro.rpc.StatusMessage.text)
}
inline void StatusMessage::set_text(const char* value) {
  
  text_.Set(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArenaNoVirtual());
  // @@protoc_insertion_point(field_set_char:vqro.rpc.StatusMessage.text)
}
inline void StatusMessage::set_text(const char* value,
    size_t size) {
  
  text_.Set(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArenaNoVirtual());
  // @@protoc_insertion_point(field_set_pointer:vqro.rpc.StatusMessage.text)
}
inline ::std::string* StatusMessage::mutable_text() {
  
  // @@protoc_insertion_point(field_mutable:vqro.rpc.StatusMessage.text)
  return text_.Mutable(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArenaNoVirtual());
}
inline ::std::string* StatusMessage::release_text() {
  
  return text_.Release(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArenaNoVirtual());
}
inline ::std::string* StatusMessage::unsafe_arena_release_text() {
  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);
  
  return text_.UnsafeArenaRelease(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      GetArenaNoVirtual());
}
inline void StatusMessage::set_allocated_text(::std::string* text) {
  if (text != NULL) {
    
  } else {
    
  }
  text_.SetAllocated(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), text,
      GetArenaNoVirtual());
  // @@protoc_insertion_point(field_set_allocated:vqro.rpc.StatusMessage.text)
}
inline void StatusMessage::unsafe_arena_set_allocated_text(
    ::std::string* text) {
  GOOGLE_DCHECK(GetArenaNoVirtual() != NULL);
  if (text != NULL) {
    
  } else {
    
  }
  text_.UnsafeArenaSetAllocated(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      text, GetArenaNoVirtual());
  // @@protoc_insertion_point(field_set_allocated:vqro.rpc.StatusMessage.text)
}

// optional bool error = 2;
inline void StatusMessage::clear_error() {
  error_ = false;
}
inline bool StatusMessage::error() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.StatusMessage.error)
  return error_;
}
inline void StatusMessage::set_error(bool value) {
  
  error_ = value;
  // @@protoc_insertion_point(field_set:vqro.rpc.StatusMessage.error)
}

// optional bool go_away = 3;
inline void StatusMessage::clear_go_away() {
  go_away_ = false;
}
inline bool StatusMessage::go_away() const {
  // @@protoc_insertion_point(field_get:vqro.rpc.StatusMessage.go_away)
  return go_away_;
}
inline void StatusMessage::set_go_away(bool value) {
  
  go_away_ = value;
  // @@protoc_insertion_point(field_set:vqro.rpc.StatusMessage.go_away)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace rpc
}  // namespace vqro

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_core_2eproto__INCLUDED
