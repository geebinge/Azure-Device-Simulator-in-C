# Azure-IoT-Device-Simulator-in-C

This is a very simple simulator for Azure IoT Hub, which you can run in a docker-compose. I mainly used and adapted 3 samples from the https://github.com/Azure/azure-iot-sdk-c/

For the device provisioning, I have adapted

  https://github.com/Azure/azure-iot-sdk-c/blob/main/provisioning_client/samples/custom_hsm_example/custom_hsm_example.c
  https://github.com/Azure/azure-iot-sdk-c/blob/main/provisioning_client/samples/prov_dev_client_sample/prov_dev_client_sample.c

For the IoT client, I have adapted

  https://github.com/Azure/azure-iot-sdk-c/blob/main/iothub_client/samples/iothub_ll_client_x509_sample/iothub_ll_client_x509_sample.c

I modified the examples so that instead of the configurations and certs in the code files, I read the config from a JSON and added the certs from files. To read and handle the JSON I used this parser in C https://github.com/DaveGamble/cJSON/

You can also find a Dockerfile if you would like to create to use your own Docker image, but you can also use the gebinger/dev_simulator:0.2 which is publicly available.

If you would like to start the simulator us

```
docker compose run -e DEVICENAME=${DEVICENAME} simulator
```
