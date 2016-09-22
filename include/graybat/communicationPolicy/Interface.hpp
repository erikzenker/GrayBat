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

namespace graybat {

    namespace communicationPolicy {

        class Interface {

            // Point to point operations
            template <typname T_Send>
            virtual void send(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData) = 0;

            template <typename T_Send>
            virtual Event asyncSend(const VAddr destVAddr, const Tag tag, const Context context, const T_Send& sendData) = 0;

            template <typename T_Recv>
            virtual void recv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData) = 0;

            template <typename T_Recv>
            virtual Event recv(const Context context, T_Recv& recvData) = 0;

            template <typename T_Recv>
            virtual Event asyncRecv(const VAddr srcVAddr, const Tag tag, const Context context, T_Recv& recvData) = 0;

            // Collective operations
            template <typename T_Send, typename T_Recv>
            virtual void gather(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv>
            virtual void gatherVar(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount) = 0;

            template <typename T_Send, typename T_Recv>
            virtual void allGather(Context context, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv>
            virtual void allGatherVar(const Context context, const T_Send& sendData, T_Recv& recvData, std::vector<unsigned>& recvCount) = 0;

            template <typename T_Send, typename T_Recv>
            virtual void scatter(const VAddr rootVAddr, const Context context, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv>
            virtual void allScatter(const Context context, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv, typename T_Op>
            virtual void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv, typename T_Op>
            virtual void reduce(const VAddr rootVAddr, const Context context, const T_Op op, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_Send, typename T_Recv, typename T_Op>
            virtual void allReduce(const Context context, T_Op op, const T_Send& sendData, T_Recv& recvData) = 0;

            template <typename T_SendRecv>
            virtual void broadcast(const VAddr rootVAddr, const Context context, T_SendRecv& data) = 0;

            virtual void synchronize(const Context context) = 0;


            // Context operations
            virtual Context splitContext(const bool isMember, const Context oldContext) = 0;

            virtual Context getGlobalContext() = 0;

        };

    } // communicationPolicy

} // graybat