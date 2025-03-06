/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const DocumentSelectMode = {
  FILE: 0,
  FOLDER: 1,
  MIXED: 2,
};

const ErrCode = {
  INVALID_ARGS: 13900020,
  RESULT_ERROR: 13900042,
  NAME_TOO_LONG: 13900030,
  CONTEXT_NO_EXIST: 16000011,
};

const ERRCODE_MAP = new Map([
  [ErrCode.INVALID_ARGS, 'Invalid argument'],
  [ErrCode.RESULT_ERROR, 'Unknown error'],
  [ErrCode.NAME_TOO_LONG, 'File name too long'],
  [ErrCode.CONTEXT_NO_EXIST, 'Current ability failed to obtain context'],
]);

const CREATE_FILE_NAME_LENGTH_LIMIT = 256;
const ARGS_ZERO = 0;
const ARGS_ONE = 1;
const ARGS_TWO = 2;
const filepickerModule = requireInternal('file.picker');

function strSizeUTF8(str) {
  let strLen = str.length;
  let bytesLen = 0;
  let greeceLen = 2;
  let chineseLen = 3;
  let othersLen = 4;
  for (let i = 0; i < strLen; i++) {
    let charCode = str.charCodeAt(i);
    if (charCode <= 0x007f) {
      bytesLen++;
    } else if (charCode <= 0x07ff) {
      bytesLen += greeceLen;
    } else if (charCode <= 0xffff) {
      bytesLen += chineseLen;
    } else {
      bytesLen += othersLen;
    }
  }
  return bytesLen;
}

function checkArguments(args) {
  let checkArgumentsResult = undefined;

  if (args.length === ARGS_TWO && typeof args[ARGS_ONE] !== 'function') {
    checkArgumentsResult = getErr(ErrCode.INVALID_ARGS);
  }

  if (args.length > 0 && typeof args[ARGS_ZERO] === 'object') {
    let option = args[ARGS_ZERO];
    if (option.maxSelectNumber !== undefined) {
      if (option.maxSelectNumber.toString().indexOf('.') !== -1) {
        checkArgumentsResult = getErr(ErrCode.INVALID_ARGS);
      }
    }

    if (option.newFileNames !== undefined && option.newFileNames.length > 0) {
      for (let i = 0; i < option.newFileNames.length; i++) {
        let value = option.newFileNames[i];
        if (strSizeUTF8(value) >= CREATE_FILE_NAME_LENGTH_LIMIT) {
          console.log(`[picker] checkArguments Invalid name: ${value}`);
          checkArgumentsResult = getErr(ErrCode.NAME_TOO_LONG);
        }
      }
    }
  }

  return checkArgumentsResult;
}

function getErr(errCode) {
  return { code: errCode, message: ERRCODE_MAP.get(errCode) };
}

function parseDocumentPickerSelectOption(args) {
  let config = {
    parameters: {},
  };

  if (args.length > ARGS_ZERO && typeof args[ARGS_ZERO] === 'object') {
    let option = args[ARGS_ZERO];

    if ((option.selectMode !== undefined)) {
      config.parameters.key_select_mode = option.selectMode;
    }
    
    if ((option.maxSelectNumber !== undefined) && option.maxSelectNumber > 0) {
      config.parameters.key_pick_num = option.maxSelectNumber;
    }
    if (option.defaultFilePathUri !== undefined) {
      config.parameters.key_pick_dir_path = option.defaultFilePathUri;
    }
    if ((option.fileSuffixFilters !== undefined) && option.fileSuffixFilters.length > 0) {
      config.parameters.key_file_suffix_filter = option.fileSuffixFilters;
    }
    if (option.authMode !== undefined) {
      config.parameters.key_auth_mode = option.authMode;
    }
  }

  console.log(`[picker] document select config: ${JSON.stringify(config)}`);
  return config;
}

async function documentPickerSelect(...args) {
  let checkDocumentSelectArgsResult = checkArguments(args);
  if (checkDocumentSelectArgsResult !== undefined) {
    console.log('[picker] Document Select Invalid argument');
    throw checkDocumentSelectArgsResult;
  }

  let documentSelectConfig = undefined;
  let documentSelectResult = {
    error: undefined,
    data: undefined,
  };

  try {
    documentSelectConfig = parseDocumentPickerSelectOption(args);
    documentSelectResult = await this.filepicker.select(documentSelectConfig.parameters);
  } catch (error) {
    console.error(`[picker] DocumentSelect error: ${JSON.stringify(error)}`);
    return undefined;
  }
  console.log(`[picker] DocumentSelect result: ${JSON.stringify(documentSelectResult)}`);
  try {
    let selectResult = {
      error: documentSelectResult.error,
      data: documentSelectResult.data,
    };
    if (args.length === ARGS_TWO && typeof args[ARGS_ONE] === 'function') {
      console.debug('[picker] select 1 called');
      return args[ARGS_ONE](selectResult.error, selectResult.data);
    } else if (args.length === ARGS_ONE && typeof args[ARGS_ZERO] === 'function') {
      console.debug('[picker] select 2 called');
      return args[ARGS_ZERO](selectResult.error, selectResult.data);
    }
    console.debug('[picker] select 3 called');
    return new Promise((resolve, reject) => {
      if (selectResult.data !== undefined) {
        resolve(selectResult.data);
      } else {
        reject(selectResult.error);
      }
    });
  } catch (resultError) {
    console.error(`[picker] Result error: ${resultError}`);
  }
  return undefined;
}

function parseDocumentPickerSaveOption(args) {
  let config = {
    parameters: {},
  };

  if (args.length > ARGS_ZERO && typeof args[ARGS_ZERO] === 'object') {
    let option = args[ARGS_ZERO];
    if ((option.newFileNames !== undefined) && option.newFileNames.length > 0) {
      config.parameters.key_pick_file_name = option.newFileNames;
      config.parameters.saveFile = option.newFileNames[0];
    }

    if (option.defaultFilePathUri !== undefined) {
      config.parameters.key_pick_dir_path = option.defaultFilePathUri;
    }
    if ((option.fileSuffixChoices !== undefined) && option.fileSuffixChoices.length > 0) {
      config.parameters.key_file_suffix_choices = option.fileSuffixChoices;
    }
  }

  console.log(`[picker] document save config: ${JSON.stringify(config)}`);
  return config;
}

async function documentPickerSave(...args) {
  let checkDocumentSaveArgsResult = checkArguments(args);
  if (checkDocumentSaveArgsResult !== undefined) {
    console.log('[picker] Document Save Invalid argument');
    throw checkDocumentSaveArgsResult;
  }

  let documentSaveConfig = undefined;
  let documentSaveResult = {
    error: undefined,
    data: undefined,
  };

  try {
    documentSaveConfig = parseDocumentPickerSaveOption(args);
    documentSaveResult = await this.filepicker.save(documentSaveConfig.parameters);
  } catch (error) {
    console.error(`[picker] document save error: ${error}`);
    return undefined;
  }

  console.log(`[picker] document save result: ${JSON.stringify(documentSaveResult)}`);
  try {
    let saveResult = {
      error: documentSaveResult.error,
      data: documentSaveResult.data,
    };
    console.log(`[picker] document saveResult: ${JSON.stringify(saveResult)}`);
    if (args.length === ARGS_TWO && typeof args[ARGS_ONE] === 'function') {
      console.debug('[picker] save 1 called');
      return args[ARGS_ONE](saveResult.error, saveResult.data);
    } else if (args.length === ARGS_ONE && typeof args[ARGS_ZERO] === 'function') {
      console.debug('[picker] save 2 called');
      return args[ARGS_ZERO](saveResult.error, saveResult.data);
    }
    console.debug('[picker] save 3 called');
    return new Promise((resolve, reject) => {
      if (saveResult.data !== undefined) {
        resolve(saveResult.data);
      } else {
        reject(saveResult.error);
      }
    });
  } catch (resultError) {
    console.error(`[picker] Result error: ${resultError}`);
  }
  return undefined;
}

function DocumentSelectOptions() {
  this.defaultFilePathUri = undefined;
  this.fileSuffixFilters = undefined;
  this.maxSelectNumber = undefined;
  this.selectMode = DocumentSelectMode.FILE;
}

function DocumentSaveOptions() {
  this.newFileNames = undefined;
  this.defaultFilePathUri = undefined;
  this.fileSuffixChoices = undefined;
}

function AudioSelectOptions() { }

function AudioSaveOptions() {
  this.newFileNames = undefined;
}

function DocumentViewPicker() {
  this.filepicker = new filepickerModule.documentViewPicker();
  this.select = documentPickerSelect;
  this.save = documentPickerSave;
}

function AudioViewPicker() {
  this.filepicker = new filepickerModule.documentViewPicker();
  this.select = documentPickerSelect;
  this.save = documentPickerSave;
}

export default {
  DocumentSelectMode,
  DocumentSelectOptions,
  DocumentSaveOptions,
  AudioSelectOptions,
  AudioSaveOptions,
  DocumentViewPicker,
  AudioViewPicker,
};