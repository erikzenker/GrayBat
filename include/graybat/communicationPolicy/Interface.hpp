/**
 * Copyright 2016 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <graybat/communicationPolicy/Traits.hpp>


namespace graybat {

    namespace communicationPolicy {

        template<typename T_CommunicationPolicy>
        class Interface {

            using CommunicationPolicy = T_CommunicationPolicy;
            using VAddr               = typename graybat::communicationPolicy::VAddr<CommunicationPolicy>;
            using Tag                 = typename graybat::communicationPolicy::Tag<CommunicationPolicy>;
            using Context             = typename graybat::communicationPolicy::Context<CommunicationPolicy>;
            using Event               = typename graybat::communicationPolicy::Event<CommunicationPolicy>;

            // Point to point operations
            template<typename T_Send>
            void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send &sendData) = delete;

            template<typename T_Send>
            Event
            asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send &sendData) = delete;

            template<typename T_Recv>
            void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv &recvData) = delete;

            template<typename T_Recv>
            Event recv(const Context context, T_Recv &recvData) = delete;

            template<typename T_Recv>
            Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv &recvData) = delete;

            // Collective operations
            template<typename T_Send, typename T_Recv>
            void
            gather(const VAddr rootVAddr, const Context context, const T_Send &sendData, T_Recv &recvData) = delete;

            template<typename T_Send, typename T_Recv>
            void gatherVar(const VAddr rootVAddr, const Context context, const T_Send &sendData, T_Recv &recvData,
                           std::vector<unsigned> &recvCount) = delete;

            template<typename T_Send, typename T_Recv>
            void allGather(Context context, const T_Send &sendData, T_Recv &recvData) = delete;

            template<typename T_Send, typename T_Recv>
            void allGatherVar(const Context context, const T_Send &sendData, T_Recv &recvData,
                              std::vector<unsigned> &recvCount) = delete;

            template<typename T_Send, typename T_Recv>
            void
            scatter(const VAddr rootVAddr, const Context context, const T_Send &sendData, T_Recv &recvData) = delete;

            template<typename T_Send, typename T_Recv>
            void allScatter(const Context context, const T_Send &sendData, T_Recv &recvData) = delete;

            template<typename T_Send, typename T_Recv, typename T_Op>
            void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send &sendData,
                        T_Recv &recvData) = delete;

            template<typename T_Send, typename T_Recv, typename T_Op>
            void allReduce(const Context context, T_Op op, const T_Send &sendData, T_Recv &recvData) = delete;

            template<typename T_SendRecv>
            void broadcast(const VAddr rootVAddr, const Context context, T_SendRecv &data) = delete;

            void synchronize(const Context context) = delete;


            // Context operations
            Context splitContext(const bool isMember, const Context oldContext) = delete;

            Context getGlobalContext() = delete;

        };

    } // communicationPolicy

} // graybat