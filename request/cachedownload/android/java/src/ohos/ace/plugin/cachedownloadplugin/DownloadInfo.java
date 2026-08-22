/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.cachedownloadplugin;

import java.util.List;

/**
 * Pre-downloaded download information.
 * Matches the ArkTS DownloadInfo interface.
 *
 * @since 1.0.0
 */
public class DownloadInfo {

    /** Pre-downloaded resource information. */
    public final ResourceInfo resource = new ResourceInfo();

    /** Pre-downloaded network information. */
    public final NetworkInfo network = new NetworkInfo();

    /** Pre-downloaded performance information. */
    public final PerformanceInfo performance = new PerformanceInfo();

    // ============================================================
    // Sub-types
    // ============================================================

    /** Resource information. */
    public static class ResourceInfo {
        /** Size after decompression in bytes. -1 = download failed. */
        private long size = -1;

        /**
         * Gets the resource size.
         *
         * @return Size in bytes, or -1 if download failed.
         */
        public long getSize() {
            return size;
        }
    }

    /** Network information. */
    public static class NetworkInfo {
        /** DNS servers used for downloading. */
        private List<String> dnsServers;

        /** IP address of the URL. Null if DNS resolution failed. */
        private String ip;

        /**
         * Gets the DNS servers.
         *
         * @return List of DNS server addresses, or null if not available.
         */
        public List<String> getDnsServers() {
            return dnsServers;
        }

        /**
         * Gets the server IP address.
         *
         * @return IP address string, or null if DNS resolution failed.
         */
        public String getIp() {
            return ip;
        }
    }

    /** Performance information. All times in milliseconds. */
    public static class PerformanceInfo {
        /** DNS resolution time in milliseconds. */
        private double dnsTime;

        /** TCP connection time in milliseconds. */
        private double connectTime;

        /** TLS handshake time in milliseconds. */
        private double tlsTime;

        /** Time until first request byte sent in milliseconds. */
        private double firstSendTime;

        /** Time until first response byte received in milliseconds. */
        private double firstRecvTime;

        /** Total request time in milliseconds. */
        private double totalTime;

        /** Redirect time in milliseconds. */
        private double redirectTime;

        /**
         * Gets the DNS resolution time.
         *
         * @return DNS time in milliseconds.
         */
        public double getDnsTime() {
            return dnsTime;
        }

        /**
         * Gets the TCP connection time.
         *
         * @return Connect time in milliseconds.
         */
        public double getConnectTime() {
            return connectTime;
        }

        /**
         * Gets the TLS handshake time.
         *
         * @return TLS time in milliseconds.
         */
        public double getTlsTime() {
            return tlsTime;
        }

        /**
         * Gets the first send time.
         *
         * @return First send time in milliseconds.
         */
        public double getFirstSendTime() {
            return firstSendTime;
        }

        /**
         * Gets the first receive time.
         *
         * @return First receive time in milliseconds.
         */
        public double getFirstRecvTime() {
            return firstRecvTime;
        }

        /**
         * Gets the total request time.
         *
         * @return Total time in milliseconds.
         */
        public double getTotalTime() {
            return totalTime;
        }

        /**
         * Gets the redirect time.
         *
         * @return Redirect time in milliseconds.
         */
        public double getRedirectTime() {
            return redirectTime;
        }
    }
}
