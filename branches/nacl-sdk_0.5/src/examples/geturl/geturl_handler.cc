// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "examples/geturl/geturl_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"

namespace {
bool IsError(int32_t result) {
  return ((PP_OK != result) && (PP_OK_COMPLETIONPENDING != result));
}
}  // namespace

GetURLHandler* GetURLHandler::Create(pp::Instance* instance,
                                     const std::string& url) {
  return new GetURLHandler(instance, url);
}

GetURLHandler::GetURLHandler(pp::Instance* instance,
                             const std::string& url)
    : instance_(instance),
      url_(url),
      url_request_(instance),
      url_loader_(instance),
      cc_factory_(this) {
  url_request_.SetURL(url);
  url_request_.SetMethod("GET");
}

GetURLHandler::~GetURLHandler() {
}

bool GetURLHandler::Start() {
  pp::CompletionCallback cc = cc_factory_.NewCallback(&GetURLHandler::OnOpen);
  int32_t res = url_loader_.Open(url_request_, cc);
  if (PP_OK_COMPLETIONPENDING != res)
    cc.Run(res);

  return !IsError(res);
}

void GetURLHandler::OnOpen(int32_t result) {
  if (result < 0)
    ReportResultAndDie(url_, "pp::URLLoader::Open() failed", false);
  else
    ReadBody();
}

void GetURLHandler::OnRead(int32_t result) {
  if (result < 0) {
    ReportResultAndDie(url_,
                       "pp::URLLoader::ReadResponseBody() result<0",
                       false);

  } else if (result != 0) {
    int32_t num_bytes = result < kBufferSize ? result : sizeof(buffer_);
    url_response_body_.reserve(url_response_body_.size() + num_bytes);
    url_response_body_.insert(url_response_body_.end(),
                              buffer_,
                              buffer_ + num_bytes);
    ReadBody();
  } else {  // result == 0, end of stream
    ReportResultAndDie(url_, url_response_body_, true);
  }
}

void GetURLHandler::ReadBody() {
  // Reads the response body (asynchronous) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  // Look at <ppapi/c/dev/ppb_url_loader> for more details.
  pp::CompletionCallback cc = cc_factory_.NewCallback(&GetURLHandler::OnRead);
  int32_t res = url_loader_.ReadResponseBody(buffer_,
                                             sizeof(buffer_),
                                             cc);
  if (PP_OK_COMPLETIONPENDING != res)
    cc.Run(res);
}

void GetURLHandler::ReportResultAndDie(const std::string& fname,
                                       const std::string& text,
                                       bool success) {
  ReportResult(fname, text, success);
  delete this;
}

void GetURLHandler::ReportResult(const std::string& fname,
                                 const std::string& text,
                                 bool success) {
  if (success)
    printf("GetURLHandler::ReportResult(Ok).\n");
  else
    printf("GetURLHandler::ReportResult(Err). %s\n", text.c_str());
  fflush(stdout);
  if (instance_) {
    pp::Var var_result(fname + "\n" + text);
    instance_->PostMessage(var_result);
  }
}
