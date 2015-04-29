#ifndef PLUGIN_STREAMBIN_H
#define PLUGIN_STREAMBIN_H

#include <SenseKit/sensekit_types.h>
#include <SenseKit/Plugins/plugin_capi.h>
#include <SenseKit/Plugins/PluginServiceProxy.h>

namespace sensekit { namespace plugins {

    template<typename TFrameType>
    class StreamBin
    {
    public:
        StreamBin(PluginServiceProxy& pluginService,
                  sensekit_stream_t streamHandle,
                  size_t dataSize)
            : m_pluginService(pluginService),
              m_streamHandle(streamHandle)
        {
            size_t dataWrapperSize = dataSize + sizeof(TFrameType);
            m_pluginService.create_stream_bin(streamHandle,
                                              dataWrapperSize,
                                              &m_binHandle,
                                              &m_currentBuffer);
        }

        ~StreamBin()
        {
            m_pluginService.destroy_stream_bin(m_streamHandle, &m_binHandle, &m_currentBuffer);
        }

        bool has_connections()
        {
            bool hasConnections = false;
            m_pluginService.bin_has_connections(m_binHandle, &hasConnections);

            return hasConnections;
        }

        void cycle()
        {
            m_pluginService.cycle_bin_buffers(m_binHandle, &m_currentBuffer);
        }

        TFrameType* begin_write(size_t frameIndex)
        {
            if (m_locked)
                return reinterpret_cast<TFrameType*>(m_currentBuffer->data);

            m_locked = true;
            m_currentBuffer->frameIndex = frameIndex;
            return reinterpret_cast<TFrameType*>(m_currentBuffer->data);
        }

        void end_write()
        {
            if (!m_locked)
                return;

            cycle();
            m_locked = false;
        }

        void link_connection(sensekit_streamconnection_t connection)
        {
            m_pluginService.link_connection_to_bin(connection, m_binHandle);
        }

        void unlink_connection(sensekit_streamconnection_t connection)
        {
            m_pluginService.link_connection_to_bin(connection, nullptr);
        }

    private:
        sensekit_stream_t m_streamHandle;
        sensekit_bin_t m_binHandle;
        size_t m_bufferSize{0};
        sensekit_frame_t* m_currentBuffer{nullptr};
        PluginServiceProxy& m_pluginService;
        bool m_locked{false};

    };

}}

#endif /* PLUGIN_STREAMBIN_H */