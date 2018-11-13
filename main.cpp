#include <QCoreApplication>
#include <QDebug>
#include "open62541.h"
#include "/usr/include/signal.h"
#include "/usr/include/wiringPi.h"
#include "/usr/include/wiringPiI2C.h"
#include "/usr/include/pcf8574.h"
#include "/usr/include/time.h"

#define ADDRESS_ANALOG_INPUT 0x18
#define ADDRESS_DIGITAL_OUTPUT 0x25
#define DIGITAL_OUTPUT_BASE 100
#define AN0 0
#define DO0 0
#define DO1 1
#define BASE 120

int fd_analog_input = 0;
int fd_digital_output = 0;


//DIGITAL OUTPUT

void digitalOutputSetUp(){

    fd_digital_output=pcf8574Setup(DIGITAL_OUTPUT_BASE,ADDRESS_DIGITAL_OUTPUT);

    for(int i=0;i<8;i++){
        pinMode(DIGITAL_OUTPUT_BASE+i,OUTPUT);
    }
    for(int i=0;i<8;i++){
        digitalWrite(DIGITAL_OUTPUT_BASE+i,1);
    }

}

static UA_StatusCode
readCurrentDigitalOutput(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean sourceTimeStamp, const UA_NumericRange *range,
                UA_DataValue *dataValue) {

    UA_Boolean statusDO2;
    if(digitalRead(DIGITAL_OUTPUT_BASE+2))
        statusDO2=false;
    else
        statusDO2=true;

    UA_Variant_setScalarCopy(&dataValue->value, &statusDO2,
                                 &UA_TYPES[UA_TYPES_BOOLEAN]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}


static UA_StatusCode
writeCurrentDigitalOutput(UA_Server *server,
                 const UA_NodeId *sessionId, void *sessionContext,
                 const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data) {

    qDebug()<<data;
    qDebug()<<data->status;
    qDebug()<<data->value.data;

    if(digitalRead(DIGITAL_OUTPUT_BASE+2)){
        digitalWrite(DIGITAL_OUTPUT_BASE+2,0);
    }else{
        digitalWrite(DIGITAL_OUTPUT_BASE+2,1);
    }

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "Digital Output  2 changed");


    return UA_STATUSCODE_GOOD;
}

static void addDigitalOutputVariable(UA_Server *server) {
        UA_NodeId digitalOutputId; /* get the nodeid assigned by the server */
        UA_NodeId currentNodeId = UA_NODEID_NULL;
        UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT);
        UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);


        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Digital Output Node");
        UA_Server_addObjectNode(server, UA_NODEID_NULL,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                UA_QUALIFIEDNAME(1, "Digital Output Node"), UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),
                                oAttr, NULL, &digitalOutputId);

        UA_VariableAttributes statuDO0Attr = UA_VariableAttributes_default;
        UA_Boolean statusDO0 = true;
        UA_Variant_setScalar(&statuDO0Attr.value, &statusDO0, &UA_TYPES[UA_TYPES_BOOLEAN]);
        statuDO0Attr.displayName = UA_LOCALIZEDTEXT("en-US", "Status DO0");
        statuDO0Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_Server_addVariableNode(server, UA_NODEID_NULL, digitalOutputId,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                  UA_QUALIFIEDNAME(1, "Status DO0"),
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), statuDO0Attr, NULL, NULL);

        UA_VariableAttributes statusDO1Attr = UA_VariableAttributes_default;
        UA_Boolean statusDO1 = true;
        UA_Variant_setScalar(&statusDO1Attr.value, &statusDO1, &UA_TYPES[UA_TYPES_BOOLEAN]);
        statusDO1Attr.displayName = UA_LOCALIZEDTEXT("en-US", "Status DO1");
        statusDO1Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        UA_Server_addVariableNode(server, UA_NODEID_NULL, digitalOutputId,
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                  UA_QUALIFIEDNAME(1, "Status DO1"),
                                  UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), statusDO1Attr, NULL, NULL);

        UA_VariableAttributes statusDO2Attr = UA_VariableAttributes_default;
        UA_Boolean statusDO2 = false;
        UA_Variant_setScalar(&statusDO2Attr.value, &statusDO2, &UA_TYPES[UA_TYPES_BOOLEAN]);
        statusDO2Attr.displayName = UA_LOCALIZEDTEXT("en-US", "Status DO2");
        statusDO2Attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;


        UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "Status DO2");
        //UA_NodeId parentNodeId = digitalOutputId;

        UA_DataSource digitalOutputData;
        digitalOutputData.read = readCurrentDigitalOutput;
        digitalOutputData.write = writeCurrentDigitalOutput;
        UA_Server_addDataSourceVariableNode(server, currentNodeId, digitalOutputId,
                                            parentReferenceNodeId, currentName,
                                            variableTypeNodeId, statusDO2Attr,
                                            digitalOutputData, NULL, NULL);

}

//ANALOG INPUT

static void updateAnalogInput(UA_Server *server) {
    //UA_Int32 analogInteger = wiringPiI2CRead(fd_analog_input);
    UA_Int32 analogInteger = 56;
    UA_Variant value;
    UA_Variant_setScalar(&value, &analogInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "current.analog.input");
    UA_Server_writeValue(server, currentNodeId, value);
}

static void addCurrentAnalogInputVariable(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 analogInteger = 46;
    UA_Variant_setScalar(&attr.value, &analogInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US","Analog input");
    attr.displayName = UA_LOCALIZEDTEXT("en-US","Analog input");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ;

    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "current.analog.input");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "current analog input");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);

    updateAnalogInput(server);
}


static void addVariable(UA_Server *server) {
    /* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 42;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.displayName = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}

static void writeVariable(UA_Server *server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

    /* Write a different integer value */
    UA_Int32 myInteger = 43;
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId, myVar);

    /* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = myIntegerNodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv.value.hasStatus = true;
    UA_Server_write(server, &wv);

    /* Reset the variable to a good statuscode with a value */
    wv.value.hasStatus = false;
    wv.value.value = myVar;
    wv.value.hasValue = true;
    UA_Server_write(server, &wv);
}

static void writeWrongVariable(UA_Server *server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

    /* Write a string */
    UA_String myString = UA_STRING("test");
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myString, &UA_TYPES[UA_TYPES_STRING]);
    UA_StatusCode retval = UA_Server_writeValue(server, myIntegerNodeId, myVar);
    printf("Writing a string returned statuscode %s\n", UA_StatusCode_name(retval));
}


UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig *config = UA_ServerConfig_new_default();
    UA_Server *server = UA_Server_new(config);

    wiringPiSetup();
    digitalOutputSetUp();

    addDigitalOutputVariable(server);

    addCurrentAnalogInputVariable(server);

    addVariable(server);
    writeVariable(server);

    UA_StatusCode retval = UA_Server_run(server, &running);
    UA_Server_delete(server);
    UA_ServerConfig_delete(config);

    return (int)retval;
    //return a.exec();
}
