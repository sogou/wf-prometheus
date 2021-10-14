# wf-prometheus

This is a light prometheus exporter using workflow HTTP server. This project is currently in the development stage, and the first version will be released soon.

### Installation

Make sure you have already installed [Workflow](https://www.github.com/sogou/workflow)

```sh
make
```

Now you can run the default example by:

```sh
./build.make/example
```

### Check by yourself

The default port is ``8080`` and the path to collect is ``/metrics``:

```sh
curl localhost:8080/metrics
```
may get the following data, which is consistent with Prometheus data model.

```sh
# HELP workflow_metrics_count help info 1
# TYPE workflow_metrics_count gauge
workflow_metrics_count 3
# HELP request_method help info 2
# TYPE request_method counter
request_method{method="post",protocol="tcp"} 4
request_method{method="get",protocol="tcp"} 2
# HELP response_body_size help info 4
# TYPE response_body_size summary
response_body_size{quantile="0.500000"} 107
response_body_size{quantile="0.900000"} 250
response_body_size_sum 4401
response_body_size_count 20
# HELP request_latency help info 3
# TYPE request_latency histogram
request_latency_bucket{le="0.100000"} 0
request_latency_bucket{le="1.000000"} 0
request_latency_bucket{le="10.000000"} 3
request_latency_bucket{le="+Inf"} 3
request_latency_sum 22.000000
request_latency_count 3
```

### Check by Prometheus

Run a [Prometheus](https://prometheus.io) and collect the metrics data:

<img src="https://raw.githubusercontent.com/wiki/holmes1412/holmes1412/workflow-prometheus_example.png" alt="workflow-prometheus" align=center />

### Easy demo

It`s very simple to make your own ``Exporter``, try it !!!

```cpp
using namespace prometheus;                                                        
                                                                                   
int main()                                                                         
{
    VarFactory::create_gauge<int>("workflow_metrics_count", "help info");
                                                                             
    WFHttpServer server([](WFHttpTask *task) {                                     
        VarFactory::gauge<int>("workflow_metrics_count")->increase();                      
        task->get_resp()->append_output_body(VarsGlobal::expose());                
    });

    if (server.start(8080) == 0)                                                   
    {                                                                              
        getchar();                                                                 
        server.stop();                                                             
    }                                                                              
                                                                                   
    return 0;                                                                      
} 
```
