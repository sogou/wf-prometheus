#include <stdio.h>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include "Vars.h"
#include "GaugeVars.h"
#include "CounterVars.h"

#ifndef _PROMETHEUS_EXPORTER_H__
#define _PROMETHEUS_EXPORTER_H__

namespace prometheus {

void prometheus_pull(WFHttpTask *task)
{
	fprintf(stderr, "process() uri=%s\n", task->get_req()->get_request_uri());

	std::string body;
	VarsLocal::counter<int>("request_method")->add("method=\"POST\"")->increase();
	VarsLocal::counter<int>("request_method")->add("method=\"POST\"")->increase();
	VarsLocal::counter<int>("request_method")->add("method=\"GET\"")->increase();
	body += VarsGlobal::expose();

	if (strcmp(task->get_req()->get_request_uri(), "/metrics"))
		return;

	VarsLocal::var("workflow_metrics_count")->increase();
	task->get_resp()->append_output_body(std::move(body));
}

class PrometheusExporter : public WFHttpServer
{
public:
	PrometheusExporter() :
		WFHttpServer(prometheus_pull)
	{
	}
};

} // namespace prometheus

#endif

