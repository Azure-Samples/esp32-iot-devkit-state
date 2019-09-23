using System;
using System.Net;
using System.Net.Http.Headers;
using System.Net.Http.Formatting;
using Microsoft.Azure.Devices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using Newtonsoft.Json;

static RegistryManager registryManager;
static ServiceClient serviceClient;
static string connectionString = Environment.GetEnvironmentVariable("iotHubConnectionString");

// Modify the device name for your environment
static string deviceName = "";

public static async Task<HttpResponseMessage> Run(HttpRequestMessage req, TraceWriter log)
{
    HttpResponseMessage response;
    registryManager = RegistryManager.CreateFromConnectionString(connectionString);
    serviceClient = ServiceClient.CreateFromConnectionString(connectionString);
    // parse query parameter
    string action = req.GetQueryNameValuePairs()
        .FirstOrDefault(q => string.Compare(q.Key, "action", true) == 0)
        .Value;
    if (action == "get")
    {
        string callback = req.GetQueryNameValuePairs()
            .FirstOrDefault(q => string.Compare(q.Key, "callback", true) == 0)
            .Value;

        if (String.IsNullOrEmpty(callback))
        {
            callback = "callback";
        }
        var twin = await registryManager.GetTwinAsync(deviceName);
        var json = twin.Properties.Reported.ToString();
        response = new HttpResponseMessage(HttpStatusCode.OK);
        response.Content = new StringContent(callback + "(" + json + ");", System.Text.Encoding.UTF8, "application/javascript");
    }
    else if (action == "restart")
    {
        var methodInvocation = new CloudToDeviceMethod("restart") { ResponseTimeout = TimeSpan.FromSeconds(30) };
        var methodResponse = await serviceClient.InvokeDeviceMethodAsync(deviceName, methodInvocation);
        log.Verbose(methodResponse.GetPayloadAsJson());
        response = new HttpResponseMessage(HttpStatusCode.OK);
    }
    else
    {
        response = new HttpResponseMessage(HttpStatusCode.BadRequest);
    }

    return response;
}
