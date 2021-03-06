/*
  Copyright (c) 2021 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Li Yingxin (liyingxin@sogou-inc.com)
*/

#include "Exporter.h"
#include "VarFactory.h"

namespace prometheus {

void PrometheusExporter::pull(WFHttpTask *task)
{
	fprintf(stderr, "process() uri=%s\n", task->get_req()->get_request_uri());

	std::string body;
	VarFactory::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "post"}})->increase();
	VarFactory::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "post"}})->increase();
	VarFactory::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "get"}})->increase();
	VarFactory::histogram<double>("request_latency")->observe(rand() % 11);

	if (strcmp(task->get_req()->get_request_uri(), "/metrics"))
		return;

	VarFactory::gauge<int>("workflow_metrics_count")->increase();

	body = VarFactory::expose();
	task->get_resp()->append_output_body(std::move(body));

	for (int i = 1; i < 11; i ++)
		VarFactory::summary<int>("response_body_size")->observe(body.length() / i);
}

} // namespace prometheus

