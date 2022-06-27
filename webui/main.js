"use strict";
let definedParameters = [];
let definedPresets = [];
let predefinedParameters = [];
let models = [];

let boardDefaults = {};
window.addEventListener('load', async function () {

    document.getElementById('submit').addEventListener('click', sendConfigData);
    document.getElementById('btnWifiListRefresh').addEventListener('click', loadWifiNetworks);
    document.getElementById('ssid_select').addEventListener('change', selectWifiNetwork);
    
    await fetch('/loadBoardInfo', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data) {
        const pinSelects = document.querySelectorAll('select[data-pins]');

        const pins = data["Pins"];
        
        for (let key in pins) {
            if (pins.hasOwnProperty(key)) {
                pinSelects.forEach((select) => {
                    const option = document.createElement("option");
                    option.text = pins[key];
                    option.value = key;                
                    select.add(option);
                });            
            }
        };

        boardDefaults = data['Default'];
        resetToDefaults();
    })
    .catch(function(err) {
        alert('Fetching pins data failed! Message: ' + err);
    }); 

    await refreshModels();
    await loadConfig();
})

async function resetToDefaults()
{
    if(!Object.keys(boardDefaults).length)
        return;

    document.getElementById('pin_rx').value = boardDefaults['pin_rx']; 
    document.getElementById('pin_tx').value = boardDefaults['pin_tx']; 
    document.getElementById('pin_therm').value = boardDefaults['pin_therm']; 
    document.getElementById('pin_sg1').value = boardDefaults['pin_sg1']; 
    document.getElementById('pin_sg2').value = boardDefaults['pin_sg2']; 
    document.getElementById('pin_enable_config').value = boardDefaults['pin_enable_config']; 
    document.getElementById('frequency').value = boardDefaults['frequency']; 
    document.getElementById('mqtt_onetopic').value = boardDefaults['mqtt_onetopic']; 
    document.getElementById('mqtt_port').value = boardDefaults['mqtt_port']; 
}

async function loadConfig()
{
    await fetch('/loadConfig', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data){
        // if no config exists yet
        if(Object.keys(data).length == 0)
            return;
            
        document.getElementById('ssid').value = data['SSID'];
        document.getElementById('ssid_password').value = data['SSID_PASSWORD'];
        document.getElementById('ssid_staticip').checked = data['SSID_STATIC_IP'];

        if(data['SSID_STATIC_IP'])
        {        
            document.getElementById('ssid_ip').value = data['SSID_IP'];
            document.getElementById('ssid_subnet').value = data['SSID_SUBNET'];
            document.getElementById('ssid_gateway').value = data['SSID_GATEWAY'];
            document.getElementById('primary_dns').value = data['SSID_PRIMARY_DNS'];
            document.getElementById('secondary_dns').value = data['SSID_SECONDARY_DNS'];
        }

        document.getElementById('mqtt_server').value = data['MQTT_SERVER'];
        document.getElementById('mqtt_username').value = data['MQTT_USERNAME'];
        document.getElementById('mqtt_password').value = data['MQTT_PASSWORD'];
        document.getElementById('mqtt_use_onetopic').checked = data['MQTT_USE_JSONTABLE'];
        document.getElementById('mqtt_jsontable').checked = data['MQTT_USE_ONETOPIC'];

        if(data['MQTT_USE_ONETOPIC'])
        { 
            document.getElementById('mqtt_onetopic').value = data['MQTT_ONETOPIC_NAME'];
        }

        document.getElementById('mqtt_port').value = data['MQTT_PORT'];
        document.getElementById('frequency').value = data['FREQUENCY'];
        document.getElementById('pin_rx').value = data['PIN_RX'];
        document.getElementById('pin_tx').value = data['PIN_TX'];
        document.getElementById('pin_therm').value = data['PIN_THERM'];
        document.getElementById('sg_enabled').checked = data['SG_ENABLED'];
        
        if(data['SG_ENABLED'])
        { 
            document.getElementById('pin_sg1').value = data['PIN_SG1'];
            document.getElementById('pin_sg2').value = data['PIN_SG2'];
            document.getElementById('sg_relay_trigger').checked = data['SG_RELAY_HIGH_TRIGGER'];
        }
        
        document.getElementById('pin_enable_config').value = data['PIN_ENABLE_CONFIG'];   
        
        definedParameters = data['PARAMETERS'];
        addCustomOptionToPresetsList();
        updateParametersTable('selectedParametersTable', definedParameters);
        document.getElementById('presetParameters').value = 'custom';
        updateParameters();
    })
    .catch(function(err) {
        alert('Fetching config failed! Message: ' + err);
    });    
}

async function selectWifiNetwork(event)
{    
    event.preventDefault();
    
    const ssid = document.getElementById('ssid');
    const ssidSelect = document.getElementById('ssid_select');

    if(ssidSelect.value != '')
    {
        ssid.value = ssidSelect.value;
        ssidSelect.value = '';
    }
}

async function loadWifiNetworks(event)
{    
    const ssidSelect = document.getElementById('ssid_select');
    const btnWifiListRefresh = document.getElementById('btnWifiListRefresh');

    if(btnWifiListRefresh.getAttribute('aria-busy') == 'true')
        return;
    
    const btnValue = btnWifiListRefresh.text;
    btnWifiListRefresh.text = '';
    btnWifiListRefresh.setAttribute('aria-busy', 'true'); 
        
    while (ssidSelect.options.length > 1)
        ssidSelect.remove(1);

    await fetch('/loadWifiNetworks', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data) {
        for (let key in data) {
            let option = document.createElement("option");
            option.text = data[key]["SSID"] + " (Quality:" + data[key]["RSSI"] + ") " + data[key]["EncryptionType"];
            option.value = data[key]["SSID"];                
            ssidSelect.add(option);
        };
    })
    .catch(function(err) {
        alert('Fetching wifi list failed! Message: ' + err);
    })       

    btnWifiListRefresh.setAttribute('aria-busy', 'false'); 
    btnWifiListRefresh.text = btnValue;
}

async function sendConfigData(event)
{
    event.preventDefault();

    const form = document.getElementById("configForm");
    const formData = new FormData(form);

    const ssid = document.getElementById('ssid');
    ssid.setAttribute('aria-invalid', ssid.value == '');

    const ssid_staticip = document.getElementById('ssid_staticip');
    if(ssid_staticip.checked)
    {        
        const ssid_ip = document.getElementById('ssid_ip');
        ssid_ip.setAttribute('aria-invalid',  ssid_ip.value == '' || !ValidateIPaddress(ssid_ip.value));

        const ssid_subnet = document.getElementById('ssid_subnet');
        ssid_subnet.setAttribute('aria-invalid',  ssid_subnet.value == '' || !ValidateIPaddress(ssid_subnet.value));

        const ssid_gateway = document.getElementById('ssid_gateway');
        ssid_gateway.setAttribute('aria-invalid',  ssid_gateway.value == '' || !ValidateIPaddress(ssid_gateway.value));

        const primary_dns = document.getElementById('primary_dns');
        primary_dns.setAttribute('aria-invalid',  primary_dns.value != '' && !ValidateIPaddress(primary_dns.value));

        const secondary_dns = document.getElementById('secondary_dns');
        secondary_dns.setAttribute('aria-invalid',  secondary_dns.value != '' && !ValidateIPaddress(secondary_dns.value));
    }
    else
    {
        clearHiddenValidationResult('staticip');
    }

    const mqtt_server = document.getElementById('mqtt_server');
    mqtt_server.setAttribute('aria-invalid', mqtt_server.value == '' || !ValidateIPOrHostname(mqtt_server.value));

    const mqtt_username = document.getElementById('mqtt_username');
    mqtt_username.setAttribute('aria-invalid', mqtt_username.value == '');

    const mqtt_use_onetopic = document.getElementById('mqtt_use_onetopic');
    if(mqtt_use_onetopic.checked)
    { 
        const mqtt_onetopic = document.getElementById('mqtt_onetopic');
        mqtt_onetopic.setAttribute('aria-invalid', mqtt_onetopic.value == '' || !ValidateMQTTTopic(mqtt_onetopic.value));
    }
    else
    {
        clearHiddenValidationResult("onetopic");
    }
    
    const mqtt_port = document.getElementById('mqtt_port');
    mqtt_port.setAttribute('aria-invalid', mqtt_port.value == '' || mqtt_port.value < 0 || mqtt_port.value > 65535);

    const frequency = document.getElementById('frequency');
    frequency.setAttribute('aria-invalid', frequency.value == '');

    const pin_rx = document.getElementById('pin_rx');
    pin_rx.setAttribute('aria-invalid', pin_rx.value == '');

    const pin_tx = document.getElementById('pin_tx');
    pin_tx.setAttribute('aria-invalid', pin_tx.value == '');

    const pin_therm = document.getElementById('pin_therm');
    pin_therm.setAttribute('aria-invalid', pin_therm.value == '');
    
    const sg_enabled = document.getElementById('sg_enabled');
    if(sg_enabled.checked)
    { 
        const pin_sg1 = document.getElementById('pin_sg1');
        pin_sg1.setAttribute('aria-invalid', pin_sg1.value == '');

        const pin_sg2 = document.getElementById('pin_sg2');
        pin_sg2.setAttribute('aria-invalid', pin_sg2.value == '');
    }
    else
    {
        clearHiddenValidationResult("smartgrid");
    }
    
    const pin_enable_config = document.getElementById('pin_enable_config');
    pin_enable_config.setAttribute('aria-invalid', pin_enable_config.value == '');
    
    const validationErrorField = document.querySelector('[aria-invalid="true"]');
    if(validationErrorField)
    {
        validationErrorField.focus();
        await sleep(100);
        alert("Please fill in all required fields!");
        return;
    }

    formData.append("definedParameters", JSON.stringify(definedParameters));
    await fetch(form.getAttribute('action'), {
        method: form.getAttribute('method'),
        body: formData
    })
    .then(function(response) {         
        if(response.status != 200)
        {
            alert("Error saving config: " + response.data);
            return;
        }

        alert("Config successfully saved! ESP32 will restart now with new config");
    })
    .catch(function(err) {
        alert('Saving config failed! Message: ' + err);
    });    
}

function clearHiddenValidationResult(elementName)
{
    document.getElementById(elementName).querySelectorAll("[aria-invalid]").forEach((el) => el.removeAttribute('aria-invalid'));
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function ValidateIPaddress(ipaddress) 
{
    return /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipaddress);  
}

function ValidateHostname(hostname) 
{
    return /^[a-zA-Z][a-zA-Z\d-]{1,22}[a-zA-Z\d]$/.test(hostname);  
}

function ValidateIPOrHostname(input)
{
    return !(!ValidateIPaddress(input) && !ValidateHostname(input))
}

function ValidateMQTTTopic(topicName)
{
    return /^(?:(?:[a-zA-Z0-9_-]+)\/)*([a-zA-Z0-9_-]+\/)$/.test(topicName);  
}

function show(id)
{
    document.getElementById(id).classList.toggle('hidden');
}

async function updatePresets()
{
    const modelFile = document.getElementById('language').value;

    if(modelFile == '')
    {                
        definedPresets = [];
        predefinedParameters = [];
        
        const presetParametersSelect = document.getElementById('presetParameters');        

        presetParametersSelect.value = '';
        
        while (presetParametersSelect.options.length > 1)
            presetParametersSelect.remove(1);

        updateParametersTable('parametersTable', predefinedParameters);
        updateParameters();
        return;
    }

    const formData = new FormData();           
    formData.append("modelFile", modelFile);
    await fetch('/loadModel', {
        method: "POST",
        body: formData
    })
    .then(function(response) { return response.json(); })
    .then(function(data){
        definedPresets = data['Presets'];
        predefinedParameters = data['Parameters'];
        
        const presetParametersSelect = document.getElementById('presetParameters');
        
        while (presetParametersSelect.options.length > 1)
            presetParametersSelect.remove(1);

        for (let key in definedPresets) {
            if (definedPresets.hasOwnProperty(key)) {
                const option = document.createElement("option");
                option.text = key;
                option.value = JSON.stringify(definedPresets[key]);
                presetParametersSelect.add(option);
            }
        }

        const optionAll = document.createElement("option");
        optionAll.text = "All";
        optionAll.value = "all";
        presetParametersSelect.add(optionAll);

        addCustomOptionToPresetsList();
        updateParametersTable('parametersTable', predefinedParameters);
    })
    .catch(function(err) {
        alert('Fetching parameter data failed! Message: ' + err);
    });    
}

function addCustomOptionToPresetsList()
{    
    const presetParametersSelect = document.getElementById('presetParameters');

    const optionCustom = document.createElement("option");
    optionCustom.text = "Custom (advanced user)";
    optionCustom.value = "custom";
    presetParametersSelect.add(optionCustom);
}

async function updateParameters()
{
    let selectedPreset = document.getElementById('presetParameters').value;

    document.getElementById('containerCustomParameters').classList.toggle('hidden');

    definedParameters = [];

    if(selectedPreset == '' || selectedPreset == 'custom')
        return;

    selectedPreset = selectedPreset == 'all' ? [...Array(predefinedParameters.length).keys()] : JSON.parse(selectedPreset);

    selectedPreset.forEach((value) => {
        definedParameters.push(predefinedParameters[value]);
    });
}

async function uploadFile() {

    const parametersFile = document.getElementById('parametersFile');
    const file = parametersFile.files[0];

    parametersFile.setAttribute('aria-invalid', !file);
    
    if(!file)
        return;

    const formData = new FormData();           
    formData.append("file", parametersFile.files[0]);
    await fetch('/upload', {
        method: "POST", 
        body: formData
    })  
    .then(function(response) {
        if (response.status == 200) {
            parametersFile.removeAttribute('aria-invalid');
            parametersFile.value = null;
            models = [];
            refreshModels();
        }
    })
    .catch(function(err) {
        alert('File upload failed! Message: ' + err);
    });        
}

function AddParameter(offset, regid, convid, dataSize, dataType, dataName)
{
    
    for (let i in definedParameters) {
        if(definedParameters[i][0] == offset && 
           definedParameters[i][1] == regid && 
           definedParameters[i][2] == convid && 
           definedParameters[i][3] == dataSize && 
           definedParameters[i][4] == dataType)
        {
            alert("Parameter settings of '" + dataName + "' already exists. Skip adding");
            return false;
        }

        if(definedParameters[i][5] == dataName)
        {
            alert("Parameter name '" + dataName + "' already exists. Skip adding");
            return false;
        }
    }

    const dataArray = [offset, regid, convid, dataSize, dataType, dataName];

    definedParameters.push(dataArray);
    return true;
}

function addCustomParameter()
{    
    const offset = document.getElementById('offset');
    const regid = document.getElementById('regid');
    const convid = document.getElementById('convid');
    const dataSize = document.getElementById('dataSize');
    const dataType = document.getElementById('dataType');
    const dataName = document.getElementById('dataName');

    if( offset.value == '' || isNaN(offset.value) ||
        regid.value == '' || isNaN(regid.value) ||
        convid.value == '' || isNaN(convid.value) || 
        dataSize.value == '' || isNaN(dataSize.value) ||
        dataType.value == '' || isNaN(dataType.value) ||
        dataName.value.trim() == '')
    {
        alert("Please fill in all fields correctly!");
        return false;
    }

    const result = AddParameter(offset.value, regid.value, convid.value, dataSize.value, dataType.value, dataName.value.trim());

    if(!result)
        return;

    offset.value = '';
    regid.value = '';
    convid.value = '';
    dataSize.value = '';
    dataType.value = '';
    dataName.value = '';

    updateParametersTable('selectedParametersTable', definedParameters);
}

function updateParametersTable(tableId, parameters)
{
    const selectedParametersTable = document.getElementById(tableId);

    while (selectedParametersTable.rows.length > 1) {                
        selectedParametersTable.deleteRow(1);
    }

    for (let i in parameters) {
        const data = parameters[i];

        const row = selectedParametersTable.insertRow(-1);
        row.setAttribute('data-row-index', i);
        row.addEventListener("click", function(event) {selectRow(tableId, i)});
        
        const nameCell = row.insertCell(0);
       
        const offsetCell = row.insertCell(1);
        const regidCell = row.insertCell(2);
        const convidCell = row.insertCell(3);
        const dataSizeCell = row.insertCell(4);
        const dataTypeCell = row.insertCell(5);        

        nameCell.appendChild(document.createTextNode(data[5]));
        offsetCell.appendChild(document.createTextNode(data[0]));
        regidCell.appendChild(document.createTextNode(data[1]));
        convidCell.appendChild(document.createTextNode(data[2]));
        dataSizeCell.appendChild(document.createTextNode(data[3]));
        dataTypeCell.appendChild(document.createTextNode(data[4]));
        
        if(data[6] != undefined)
        {
            const valueCell = row.insertCell(6);
            valueCell.appendChild(document.createTextNode(data[6]));
        }
    }
}


function selectRow(tableid, value)
{
    const el = document
    .getElementById(tableid)
    .querySelector("[data-row-index='" + value + "']");

    const classes = el.classList;

    if(classes.contains('row-selected'))
        classes.remove('row-selected');
    else
        classes.add('row-selected');
}

function addSelectedPredefinedParameters()
{
    document
    .getElementById('parametersTable')
    .querySelectorAll(".row-selected")
    .forEach(function(e) {
        const id = parseInt(e.getAttribute('data-row-index'));
        const paramToAdd = predefinedParameters[id];

        AddParameter(paramToAdd[0], paramToAdd[1], paramToAdd[2], paramToAdd[3], paramToAdd[4], paramToAdd[5]);
        e.classList.remove('row-selected');
    });

    updateParametersTable('selectedParametersTable', definedParameters);
}

function removeSelectedParameters()
{    
    let counterRun = 0;

    document
    .getElementById('selectedParametersTable')
    .querySelectorAll(".row-selected")
    .forEach(function(e) {
        let id = parseInt(e.getAttribute('data-row-index'));
        id -= counterRun;
        definedParameters = definedParameters.filter(function(value, index){ 
            let keep = index != id;

            if(!keep)
                counterRun++;
                
            return keep;
        });
    });

    updateParametersTable('selectedParametersTable', definedParameters);
}

async function loadData(tableId)
{
    let params;
    if (tableId == 'selectedParametersTable')
        params = definedParameters;
    else
        params = predefinedParameters;

    const pinRx =  document.getElementById('pin_rx').value;
    const pinTx =  document.getElementById('pin_tx').value;

    if( pinRx == '' || isNaN(pinRx) ||
        pinTx == '' || isNaN(pinTx))
    {
        alert("Need valid PIN RX/TX to fetch values! Canceled");
        return;   
    }
        
    const formData = new FormData();           
    formData.append("PIN_RX", pinRx);
    formData.append("PIN_TX", pinTx);
    formData.append('PARAMS', JSON.stringify(params));
    await fetch('/loadValues', {
        method: "POST", 
        body: formData
    })  
    .then(function(response) { return response.json(); })
    .then(function(data){
        params.forEach((model, index) => {
            model[6] = data[index];
        });

        if (tableId == 'selectedParametersTable')
        {
            definedParameters = params;
            updateParametersTable(tableId, definedParameters);
        }
        else
        {
            predefinedParameters = params;     
            updateParametersTable(tableId, predefinedParameters);   
        }
    })
    .catch(function(err) {
        alert('Fetching param values failed! Message: ' + err);
    });    
}

async function refreshModels()
{
    await fetch('/loadModels', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data){
        models = data;

        const modelSelect = document.getElementById('model');
        
        while (modelSelect.options.length > 1)
            modelSelect.remove(1);

        data.forEach(function(model, i) {
            let option = document.createElement("option");
            option.text = model.Model;
            option.value = i;
            modelSelect.add(option);
        });
    })
    .catch(function(err) {
        alert('Fetching models data failed! Message: ' + err);
    });    
}

function refreshLanguages()
{
    const languageSelect = document.getElementById('language');
    const selectedModel = document.getElementById('model').value;
            
    while (languageSelect.options.length > 1)
        languageSelect.remove(1);

    if(selectedModel == '')
    {
        updatePresets();
        return;
    }

    const languageFiles = models[selectedModel]["Files"];
    const languageNames = Object.keys(languageFiles);

    languageNames.forEach((language, index) => {
        const option = document.createElement("option");
        option.text = language;
        option.value = languageFiles[language];
        languageSelect.add(option);
    });
}