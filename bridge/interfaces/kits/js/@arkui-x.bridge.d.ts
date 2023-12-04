/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import { AsyncCallback } from '@ohos.base';

declare namespace Bridge {
    /**
     * Bridge data type definition.
     * ArrayBuffer: Binary data transfer.
     *
     * @since 10
     */
    type S = number | boolean | string | null | ArrayBuffer;
    type T = S | Array<number> | Array<boolean> | Array<string>;
    type Message = T | Record<string, T>;
    type Parameter = Message;
    type Response = Message;
    type ResultValue = T | Map<string, T>;

    /**
     * Data transmission encoding type.
     *
     * @since 10
     */
    export enum BridgeType {
        /**
         * Json encoding mode, default.
         *
         * @since 10
         */
	 JSON_TYPE = 0,

        /**
         * Binary stream encoding.
         *
         * @since 10
         */
        BINARY_TYPE = 1
    }

    /**
     * Creates the bridge and returns the bridge object.
     *
     * @param bridgeName Unique bridge name.
     * @return Bridge object.
     * @since 10
     */
    function createBridge(bridgeName: string): BridgeObject;

    /**
     * Creates the bridge and returns the bridge object.
     *
     * @param bridgeName Unique bridge name.
     * @param type Data encoding type.
     * @return Bridge object.
     * @since 10
     */
    function createBridge(bridgeName: string, type: BridgeType): BridgeObject;

    /**
     * Method or event interface.
     *
     * @since 10
     */
    export interface MethodData {
        /**
         * Method or event name.
         *
         * @since 10
        */
        name: string;

        /**
         * @param Method parameter.
         * @return ResultValue the js-side method return value.
         * @since 10
         * @crossplatform
         */
        method: (parameters?: Record<string , Parameter>) => ResultValue;
    }

    export interface BridgeObject {
        /**
         * Returns the bridge name provided when created by createPluginBridge.
         *
         * @since 10
         */
        bridgeName: string;

        /**
         * Invoke platform-side methods.
         * @param methodName The name of the called platform side method.
         * @param parameters Platform method parameters to be called.
         * @param callback The value returned by the called platform method.
         * @return Returns the platform-side method return value.
         * @since 10
         */
        callMethod(methodName: string, parameters?: Record<string, Parameter>): Promise<ResultValue>;
        callMethod(methodName: string, ...parameters: Array<any>): Promise<ResultValue>;

        /**
         * Register JS side methods for platform side calls.
         *
         * @param method Method for platform side invocation.
         * @since 10
         */
        registerMethod(method: MethodData, callback: AsyncCallback<void>): void;
        registerMethod(method: MethodData): Promise<void>;

        /**
         * UnRegister JS side event.
         *
         * @param methodName The name of JS side event.
         * @since 10
         */
        unRegisterMethod(methodName: string, callback: AsyncCallback<void>): void;
        unRegisterMethod(methodName: string): Promise<void>;

        /**
         * JS sends messages to the platform side.
         *
         * @param message The message sent by the JS side.
         * @since 10
         */
        sendMessage(message: Message, callback: AsyncCallback<Response>): void;
        sendMessage(message: Message): Promise<Response>;

        /**
         * Receive messages from the platform.
         *
         * @param message The message sent by the platform side.
         * @since 10
         */
        setMessageListener(callback: (message: Message) => Response);

        /**
         * Register a callback for platform side calls and call platform side functions.
         * 
         * @param methodName:The name of the called platform side method.
         * @param method: Functions defined on the JS side for platform side calls.
         * @param parameters: Platform method parameters to be called.
         * @since 11
         */
        callMethodWithCallback(methodName: string, method: (parameters?: Record<string , Parameter>) => ResultValue,
        parameters?: Record<string, Parameter>): Promise<ResultValue>;
        callMethodWithCallback(methodName: string, method: (parameters?: Record<string , Parameter>) => ResultValue,
        parameters?: Array<any>): Promise<ResultValue>;
    }
}
export default Bridge;
