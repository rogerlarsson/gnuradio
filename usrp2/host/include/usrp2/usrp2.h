/* -*- c++ -*- */
/*
 * Copyright 2008 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_USRP2_H
#define INCLUDED_USRP2_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <vector>
#include <complex>
//#include <iosfwd>
#include <usrp2/rx_sample_handler.h>
#include <usrp2/tune_result.h>
#include <usrp2/rx_sample_handler.h>


namespace usrp2 {

  /*!
   * Structure to hold properties of USRP2 hardware devices.
   *
   */
  struct props
  {
    std::string addr;
    uint16_t hw_rev;
    uint8_t fpga_md5sum[16];
    uint8_t sw_md5sum[16];
  };

  typedef std::vector<props> props_vector_t;

  /*!
   * \brief Search the ethernet for all USRP2s or for a specific USRP2.
   *
   * \param ifc is the name of the OS ethernet interface (e.g., "eth0")
   * \param mac_addr is the MAC address of the desired USRP2, or "" to search for all.
   * mac_addr must be either a zero length string, "", or must be of the form
   * "01:02:03:04:05:06" or "05:06".
   *
   * \returns a vector of properties, 1 entry for each matching USRP2 found.
   */
  props_vector_t find(const std::string &ifc, const std::string &mac_addr=""); 

  class tune_result;
  
  class usrp2 : boost::noncopyable
  {
  public:
    static const unsigned int MAX_CHAN = 30;

    /*!
     * Shared pointer to this class
     */ 
    typedef boost::shared_ptr<usrp2> sptr;
 
    /*! 
     * Static function to return an instance of usrp2 as a shared pointer
     *
     * \param ifc   Network interface name, e.g., "eth0"
     * \param addr  Network mac address, e.g., "01:23:45:67:89:ab", "89:ab" or "".
     *              If \p addr is HH:HH, it's treated as if it were 00:50:c2:85:HH:HH
     *              "" will autoselect a USRP2 if there is only a single one on the local ethernet.
     */
    static sptr make(const std::string &ifc, const std::string &addr="");

    /*!
     * Class destructor
     */
    ~usrp2();  

    /*!
     * Returns the MAC address associated with this USRP
     */
    std::string mac_addr();

    /*!
     * Burn new mac address into EEPROM on USRP2
     *
     * \param new_addr  Network mac address, e.g., "01:23:45:67:89:ab" or "89:ab".
     *                  If \p addr is HH:HH, it's treated as if it were 00:50:c2:85:HH:HH
     */
    bool burn_mac_addr(const std::string &new_addr);

    /*
     * ----------------------------------------------------------------
     * Rx configuration and control
     * ----------------------------------------------------------------
     */

    /*!
     * Set receiver gain
     */
    bool set_rx_gain(double gain);

    /*!
     * Set receiver center frequency
     */
    bool set_rx_center_freq(double frequency, tune_result *result);

    /*!
     * Set receiver sample rate decimation
     */
    bool set_rx_decim(int decimation_factor);

    /*!
     * Set receiver IQ magnitude scaling
     */
    bool set_rx_scale_iq(int scale_i, int scale_q);

    /*!
     * Set received sample format
     *   
     *    domain: complex or real
     *      type: floating, fixed point, or raw
     *     depth: bits per sample
     *
     * Sets format over the wire for samples from USRP2.
     */
    // bool set_rx_format(...);

    /*!
     * Start streaming receive mode.  USRP2 will send a continuous stream of
     * DSP pipeline samples to host.  Call rx_samples(...) to access.
     * 
     * \param channel          Stream channel number (0-30)
     * \param items_per_frame  Number of 32-bit items per frame.
     */
    bool start_rx_streaming(unsigned int channel=0, unsigned int items_per_frame=0);
  
    /*!
     * Stop streaming receive mode.
     */
    bool stop_rx_streaming(unsigned int channel=0);

    /*!
     * \brief Receive data from the specified channel
     * This method is used to receive all data: streaming or discrete.
     */
    bool rx_samples(unsigned int channel, rx_sample_handler *handler);

    /*!
     * Returns number of times receive overruns have occurred
     */
    unsigned int rx_overruns();
    
    /*!
     * Returns total number of missing frames from overruns.
     */
    unsigned int rx_missing();

    /*
     * ----------------------------------------------------------------
     * Tx configuration and control
     * ----------------------------------------------------------------
     */

    /*!
     * Set transmitter gain
     */
    bool set_tx_gain(double gain);

    /*!
     * Set transmitter center frequency
     */
    bool set_tx_center_freq(double frequency, tune_result *result);

    /*!
     * Set transmitter sample rate interpolation
     */
    bool set_tx_interp(int interpolation_factor);

    /*!
     * Set transmit IQ magnitude scaling
     */
    bool set_tx_scale_iq(int scale_i, int scale_q);

    /*!
     * Set transmit sample format
     *   
     *    domain: complex or real
     *      type: floating, fixed point, or raw
     *     depth: bits per sample
     *
     * Sets format over the wire for samples to USRP2.
     */
    // bool set_tx_format(...);

    /*!
     * \brief transmit complex<float> samples to USRP2
     *
     * \param channel specifies the channel to send them to
     * \param samples are the samples to transmit.  They should be in the range [-1.0, +1.0]
     * \param nsamples is the number of samples to transmit
     * \param metadata provides the timestamp and flags
     *
     * The complex<float> samples are converted to the appropriate 
     * "on the wire" representation, depending on the current USRP2
     * configuration.  Typically, this is big-endian 16-bit I & Q.
     */
    bool tx_32fc(unsigned int channel,
		 const std::complex<float> *samples,
		 size_t nsamples,
		 const tx_metadata *metadata);

    /*!
     * \brief transmit complex<int16_t> samples to USRP2
     *
     * \param channel specifies the channel to send them to
     * \param samples are the samples to transmit
     * \param nsamples is the number of samples to transmit
     * \param metadata provides the timestamp and flags
     *
     * The complex<int16_t> samples are converted to the appropriate
     * "on the wire" representation, depending on the current USRP2
     * configuration.  Typically, this is big-endian 16-bit I & Q.
     */
    bool tx_16sc(unsigned int channel,
		 const std::complex<int16_t> *samples,
		 size_t nsamples,
		 const tx_metadata *metadata);

    /*!
     * \brief transmit raw uint32_t data items to USRP2
     *
     * The caller is responsible for ensuring that the items are
     * formatted appropriately for the USRP2 and its configuration.
     * This method is used primarily by the system itself.  Users
     * should call tx_32fc or tx_16sc instead.
     *
     * \param channel specifies the channel to send them to
     * \param items are the data items to transmit
     * \param nitems is the number of items to transmit
     * \param metadata provides the timestamp and flags
     */
    bool tx_raw(unsigned int channel,
		const uint32_t *items,
		size_t nitems,
		const tx_metadata *metadata);

    // ----------------------------------------------------------------

    /*!
     * \brief MIMO configuration
     *
     * \param flags from usrp2_mimo_config.h
     *
     * <pre>
     *   one of these:
     *
     *     MC_WE_DONT_LOCK
     *     MC_WE_LOCK_TO_SMA
     *     MC_WE_LOCK_TO_MIMO
     *
     *   and optionally this:
     *
     *     MC_PROVIDE_CLK_TO_MIMO
     * </pre>
     */
    bool config_mimo(int flags);

    class impl;		// implementation details

  private:
    // Static function to retrieve or create usrp2 instance
    static sptr find_existing_or_make_new(const std::string &ifc, props *p);

    // Only class members can instantiate this class
    usrp2(const std::string &ifc, props *p);
  
    // All private state is held in opaque pointer
    std::auto_ptr<impl> d_impl;
  };

};

std::ostream& operator<<(std::ostream &os, const usrp2::props &x);


#endif /* INCLUDED_USRP2_H */
