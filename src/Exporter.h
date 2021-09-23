#include <stdio.h>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include "Vars.h"
#include "GaugeVars.h"
#include "CounterVars.h"

#ifndef _PROMETHEUS_EXPORTER_H_
#define _PROMETHEUS_EXPORTER_H_

namespace prometheus {

static void pull(WFHttpTask *task)
{
	fprintf(stderr, "process() uri=%s\n", task->get_req()->get_request_uri());

	std::string body;
	VarsLocal::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "post"}})->increase();
	VarsLocal::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "post"}})->increase();
	VarsLocal::counter<int>("request_method")->add({{"protocol", "tcp"}, {"method", "get"}})->increase();

	if (strcmp(task->get_req()->get_request_uri(), "/metrics"))
		return;

	VarsLocal::var("workflow_metrics_count")->increase();
	body += VarsGlobal::expose();
	task->get_resp()->append_output_body(std::move(body));
}

class PrometheusExporter : public WFHttpServer
{
public:
	PrometheusExporter() :
		WFHttpServer(pull)
	{
	}
};

} // namespace prometheus

#endif

