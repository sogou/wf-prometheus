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

#include <stdio.h>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include "wf-prometheus/Exporter.h"

using namespace prometheus;

int main()
{
	VarFactory::create_gauge<int>("workflow_metrics_count", "help info 1");
	VarFactory::create_counter<int>("request_method", "help info 2");
	VarFactory::create_histogram<double>("request_latency", "help info 3",
										{0.1, 1.0, 10.0});

	VarFactory::create_summary<int>("response_body_size", "help info 4",
									{{0.5, 0.05}, {0.9, 0.01}});

	PrometheusExporter server;
	if (server.start(8080) == 0)
	{
		getchar();
		server.stop();
	}

	return 0;
}

