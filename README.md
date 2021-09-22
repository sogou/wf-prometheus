# workflow-prometheus

This is a light prometheus exporter using workflow HTTP server.

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
workflow_metrics_count 79
# HELP request_method help info 2
# TYPE request_method counter
request_method{method="POST"} 150
request_method{method="GET"} 75
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
    GaugeVars<int> gauge("workflow_metrics_count", "help info");                   
                                                                                   
    WFHttpServer server([](WFHttpTask *task) {                                     
        VarsLocal::var("workflow_metrics_count")->increase();                      
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
