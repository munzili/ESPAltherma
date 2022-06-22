var definedParameters = [];
var definedPresets = [];
var predefinedParameters = [];
var models = [];

window.addEventListener('load', async function () {
    await fetch('/loadPins', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data) {
        models = data;

        let pinSelects = document.querySelectorAll('select[data-pins]');
        
        for (var key in models) {
            if (models.hasOwnProperty(key)) {
                pinSelects.forEach((select) => {
                    let option = document.createElement("option");
                    option.text = models[key];
                    option.value = key;                
                    select.add(option);
                });            
            }
        };
    })
    .catch(function(err) {
        alert('Fetching pins data failed! Message: ' + err);
    }); 
})

function show(id)
{
    var el = document.getElementById(id).style;

    if(el.display == 'none')
        el.display = 'block';
    else
        el.display = 'none';
}

async function updatePresets()
{
    var modelFile = document.getElementById('language').value;

    let formData = new FormData();           
    formData.append("modelFile", modelFile);
    await fetch('/loadModel', {
        method: "POST",
        body: formData
    })
    .then(function(response) { return response.json(); })
    .then(function(data){
        definedPresets = data['Presets'];
        predefinedParameters = data['Parameters'];
        
        let presetParametersSelect = document.getElementById('presetParameters');
        
        while (presetParametersSelect.options.length > 1)
            presetParametersSelect.remove(1);

        for (var key in definedPresets) {
            if (definedPresets.hasOwnProperty(key)) {
                let option = document.createElement("option");
                option.text = key;
                option.value = JSON.stringify(definedPresets[key]);
                presetParametersSelect.add(option);
            }
        }

        let optionAll = document.createElement("option");
        optionAll.text = "All";
        optionAll.value = "all";
        presetParametersSelect.add(optionAll);

        let optionCustom = document.createElement("option");
        optionCustom.text = "Custom (advanced user)";
        optionCustom.value = "custom";
        presetParametersSelect.add(optionCustom);

        updateParametersTable('parametersTable', predefinedParameters);
    })
    .catch(function(err) {
        alert('Fetching parameter data failed! Message: ' + err);
    });    
}

async function updateParameters()
{
    let selectedPreset = document.getElementById('presetParameters').value;

    document.getElementById('containerCustomParameters').style.display = selectedPreset == 'custom' ? 'block' : 'none';

    definedParameters = [];

    if(selectedPreset == '')
        return;

    selectedPreset = selectedPreset == 'all' ? [...Array(predefinedParameters.length).keys()] : JSON.parse(selectedPreset);

    selectedPreset.forEach((value) => {
        definedParameters.push(predefinedParameters[value]);
    });
}

async function uploadFile() {
    let file = parametersFile.files[0];

    if(!file)
        return;

    let formData = new FormData();           
    formData.append("file", parametersFile.files[0]);
    await fetch('/upload', {
        method: "POST", 
        body: formData
    })  
    .then(function(response) {
        if (response.status !== 200) {
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

    var dataArray = [offset, regid, convid, dataSize, dataType, dataName];

    definedParameters.push(dataArray);
    return true;
}

function addCustomParameter()
{    
    let offset = document.getElementById('offset');
    let regid = document.getElementById('regid');
    let convid = document.getElementById('convid');
    let dataSize = document.getElementById('dataSize');
    let dataType = document.getElementById('dataType');
    let dataName = document.getElementById('dataName');

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

    let result = AddParameter(offset.value, regid.value, convid.value, dataSize.value, dataType.value, dataName.value.trim());

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
    let selectedParametersTable = document.getElementById(tableId);

    while (selectedParametersTable.rows.length > 1) {                
        selectedParametersTable.deleteRow(1);
    }

    for (let i in parameters) {
        let data = parameters[i];

        let row = selectedParametersTable.insertRow(-1);
        row.setAttribute('data-row-index', i);
        row.addEventListener("click", function(event) {selectRow(tableId, i)});
        
        let nameCell = row.insertCell(0);
       
        let offsetCell = row.insertCell(1);
        let regidCell = row.insertCell(2);
        let convidCell = row.insertCell(3);
        let dataSizeCell = row.insertCell(4);
        let dataTypeCell = row.insertCell(5);        

        nameCell.appendChild(document.createTextNode(data[5]));
        offsetCell.appendChild(document.createTextNode(data[0]));
        regidCell.appendChild(document.createTextNode(data[1]));
        convidCell.appendChild(document.createTextNode(data[2]));
        dataSizeCell.appendChild(document.createTextNode(data[3]));
        dataTypeCell.appendChild(document.createTextNode(data[4]));
        
        if(data[6] != undefined)
        {
            let valueCell = row.insertCell(6);
            valueCell.appendChild(document.createTextNode(data[6]));
        }
    }
}


function selectRow(tableid, value)
{
    let el = document
    .getElementById(tableid)
    .querySelector("[data-row-index='" + value + "']");

    let classes = el.classList;

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
        let id = parseInt(e.getAttribute('data-row-index'));
        let paramToAdd = predefinedParameters[id];

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
    if (tableId == 'selectedParametersTable')
        var params = definedParameters;
    else
        var params = predefinedParameters;

    let pinRx =  document.getElementById('pin_rx').value;
    let pinTx =  document.getElementById('pin_tx').value;

    if( pinRx == '' || isNaN(pinRx) ||
        pinTx == '' || isNaN(pinTx))
    {
        alert("Need valid PIN RX/TX to fetch values! Canceled");
        return;   
    }
        
    let formData = new FormData();           
    formData.append("PIN_RX", pinRx);
    formData.append("PIN_TX", pinTx);
    formData.append('PARAMS', JSON.stringify(params));
    await fetch('/loadValues', {
        method: "POST", 
        body: formData
    })  
    .then(function(response) { return response.json(); })
    .then(function(data){
        let table = document.getElementById(tableid);

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
    if(models.length > 0)
        return;

    await fetch('/loadModels', {
        method: "GET"
    })
    .then(function(response) { return response.json(); })
    .then(function(data){
        models = data;

        let modelSelect = document.getElementById('model');
        
        while (modelSelect.options.length > 1)
            modelSelect.remove(1);

        models.forEach(function(model, i) {
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
    let languageSelect = document.getElementById('language');
    let selectedModel = document.getElementById('model').value;
            
    while (languageSelect.options.length > 1)
        languageSelect.remove(1);

    let languageFiles = models[selectedModel]["Files"];

    const languageNames = Object.keys(languageFiles);

    languageNames.forEach((language, index) => {
        let option = document.createElement("option");
        option.text = language;
        option.value = languageFiles[language];
        languageSelect.add(option);
    });
}