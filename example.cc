#include <stdio.h>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include "workflow-prometheus/Exporter.h"

using namespace prometheus;

int main()
{
	GaugeVars<int> gauge("workflow_metrics_count", "help info 1");
	CounterVars<int> count("request_method", "help info 2");

	PrometheusExporter server;
    if (server.start(8080) == 0)
	{
        getchar();
        server.stop();
    }

	return 0;
}

