#ifndef VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_

#include <cstdint>
#include <string>

#include "vda5050_core/order_execution/Order.hpp"

class IStateManager
{
public:
  virtual ~IStateManager() = default;

  /// \brief Returns the nodeId of the last node that the vehicle traversed
  /// 
  /// \return String of the last traversed node's nodeId
  virtual std::string last_node_id() const = 0;
  
  /// \brief Returns the sequenceId of the last node that the vehicle traversed
  ///
  /// \return uint32 of the last traversed node's sequenceId 
  virtual uint32_t last_node_sequence_id() const = 0;

  /// \brief Query if the state manager's nodeStates array is empty 
  /// 
  /// \return true if empty
  virtual bool node_states_empty() const = 0;
  
  /// \brief Query if the state manager's actionStates array contains actions other than 'FINISHED' or 'FAILED'
  ///
  /// \return true if it contains such actions 
  virtual bool action_states_still_executing() const = 0;

  /// \brief Function to clear any existing order on the vehicle
  virtual void cleanup_previous_order() = 0;
  
  /// \brief Function to set a new order on the vehicle
  virtual void set_new_order(const vda5050_core::order::Order& order) = 0;
  
  /// \brief Function to clear any horizon edges or nodes from the state manager's edgeStates array and nodeStates array
  virtual void clear_horizon() = 0;

  /// \brief Function to append order_update to the vehicle's current order, modifying state manager's edgeStates array and nodeStates array
  ///
  /// \param order_update The incoming order update
  virtual void append_states_for_update(
    const vda5050_core::order::Order& order_update) = 0;

  /// \brief Realised that this will have the same functionality as append_states_for_update, so refactor to use that instead of this
  /// 
  /// \param order_update 
  virtual void update_current_order(
    const vda5050_core::order::Order& order_update) = 0;
};

#endif  // VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_
