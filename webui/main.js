var definedParameters = [];
var models = [];

function show(id)
{
    var el = document.getElementById(id).style;

    if(el.display == 'none')
        el.display = 'block';
    else
        el.display = 'none';
}

async function updateParameters()
{
    var model = document.getElementById('model').value;
    var language = document.getElementById('language').value;
    var parameters = document.getElementById('parameters');

    if(model != "" && language != "")
    {
        while (parameters.options.length > 0) {                
            parameters.remove(0);
    }    

    let formData = new FormData();           
    formData.append("model", model);
    formData.append("language", language);
    await fetch('/loadParameters', {
        method: "POST",
        headers: {
            "Content-type": "application/x-www-form-urlencoded; charset=UTF-8"
        },
        body: formData
    })
    .then(function(response){
        if(response.status == 200) {
            alert(response.responseText);
        }
    }
    .catch(function(err) {
        alert('Fetching parameter data failed! Message: ' + err);
    }));
    }
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
            refreshModels();
        }
    })
    .catch(function(err) {
        alert('File upload failed! Message: ' + err);
    });        
}

function addCustomParameter()
{
    let convid = document.getElementById('convid');
    let offset = document.getElementById('offset');
    let dataSize = document.getElementById('dataSize');
    let dataType = document.getElementById('dataType');
    let dataName = document.getElementById('dataName');

    if(convid.value == '' || isNaN(convid.value) || 
        offset.value == '' || isNaN(offset.value) ||
        dataSize.value == '' || isNaN(dataSize.value) ||
        dataType.value == '' || isNaN(dataType.value) ||
        dataName.value.trim() == '')
        {
            alert("Please fill in all fields correctly!");
            return false;
        }

    for (let i in definedParameters) {
        if(definedParameters[i][0] == convid.value && 
           definedParameters[i][1] == offset.value && 
           definedParameters[i][2] == dataSize.value && 
           definedParameters[i][3] == dataType.value)
        {
            alert("Parameter settings of '" + dataName.value + "' already exists. Skip adding");
            return false;
        }

        if(definedParameters[i][4] == dataName.value)
        {
            alert("Parameter name '" + dataName.value + "' already exists. Skip adding");
            return false;
        }
    }

    var dataArray = [convid.value, offset.value, dataSize.value, dataType.value, dataName.value.trim()];

    definedParameters.push(dataArray);

    convid.value = '';
    offset.value = '';
    dataSize.value = '';
    dataType.value = '';
    dataName.value = '';

    updateDefinedParametersList();
}

function updateDefinedParametersList()
{
    let selectedParametersTable = document.getElementById('selectedParametersTable');

    while (selectedParametersTable.rows.length > 1) {                
        selectedParametersTable.deleteRow(1);
    }

    for (let i in definedParameters) {
        let data = definedParameters[i];

        let row = selectedParametersTable.insertRow(-1);
        row.setAttribute('data-row-index', i);
        row.addEventListener("click", function(event) {selectRow('selectedParametersTable', i)});
        
        let nameCell = row.insertCell(0);
        let convidCell = row.insertCell(1);
        let offsetCell = row.insertCell(2);
        let dataSizeCell = row.insertCell(3);
        let dataTypeCell = row.insertCell(4);        

        nameCell.appendChild(document.createTextNode(data[4]));
        convidCell.appendChild(document.createTextNode(data[0]));
        offsetCell.appendChild(document.createTextNode(data[1]));
        dataSizeCell.appendChild(document.createTextNode(data[2]));
        dataTypeCell.appendChild(document.createTextNode(data[3]));
        
        if(data[5] != undefined)
        {
            let valueCell = row.insertCell(5);
            valueCell.appendChild(document.createTextNode(data[5]));
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

function removeSelectedParameters()
{    
    document
    .getElementById('selectedParametersTable')
    .querySelectorAll(".row-selected")
    .forEach(function(e) {
        let id = parseInt(e.getAttribute('data-row-index'));
        definedParameters = definedParameters.splice(id, 1);
    });

    updateDefinedParametersList();
}

function loadData(tableId)
{
    
}

async function refreshModels()
{
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
            option.text = model.Name;
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

    for (const [languageName, parametersFile] of Object.entries(models[selectedModel].Files)) {
        let option = document.createElement("option");
        option.text = languageName;
        option.value = parametersFile;
        languageSelect.add(option);
    }
}