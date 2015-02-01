#include "kinputmanager.h"
#include <algorithm>
#include <vector>
#include <QCursor>

/*******************************************************************************
 * Static Helper Structs
 ******************************************************************************/
template <typename T>
struct InputInstance : std::pair<T, KInputManager::InputState>
{
  typedef std::pair<T, KInputManager::InputState> base_class;
  inline InputInstance(T value) : base_class(value, KInputManager::InputInvalid) {}
  inline InputInstance(T value, KInputManager::InputState state) : base_class(value, state) {}
  inline bool operator==(const InputInstance &rhs) const
  {
    return this->first == rhs.first;
  }
};

// Instance types
typedef InputInstance<Qt::Key> KeyInstance;
typedef InputInstance<Qt::MouseButton> ButtonInstance;

// Container types
typedef std::vector<KeyInstance> KeyContainer;
typedef std::vector<ButtonInstance> ButtonContainer;

// Globals
static KeyContainer sg_keyInstances;
static ButtonContainer sg_buttonInstances;
static KPoint sg_mouseCurrPosition;
static KPoint sg_mousePrevPosition;
static KPoint sg_mouseDelta;

/*******************************************************************************
 * Static Helper Fucntions
 ******************************************************************************/
static inline KeyContainer::iterator FindKey(Qt::Key value)
{
  return std::find(sg_keyInstances.begin(), sg_keyInstances.end(), value);
}

static inline ButtonContainer::iterator FindButton(Qt::MouseButton value)
{
  return std::find(sg_buttonInstances.begin(), sg_buttonInstances.end(), value);
}

template <typename TPair>
static inline void UpdateStates(TPair &instance)
{
  switch (instance.second)
  {
  case KInputManager::InputRegistered:
    instance.second = KInputManager::InputTriggered;
    break;
  case KInputManager::InputTriggered:
    instance.second = KInputManager::InputPressed;
    break;
  case KInputManager::InputUnregistered:
    instance.second = KInputManager::InputReleased;
    break;
  default:
    break;
  }
}

template <typename TPair>
static inline bool CheckReleased(const TPair &instance)
{
  return instance.second == KInputManager::InputReleased;
}

template <typename Container>
static inline void Update(Container &container)
{
  typedef typename Container::iterator Iter;
  typedef typename Container::value_type TPair;

  // Remove old data
  Iter remove = std::remove_if(container.begin(), container.end(), &CheckReleased<TPair>);
  container.erase(remove, container.end());

  // Update existing data
  std::for_each(container.begin(), container.end(), &UpdateStates<TPair>);
}

/*******************************************************************************
 * QInput Implementation
 ******************************************************************************/
KInputManager::InputState KInputManager::keyState(Qt::Key key)
{
  KeyContainer::iterator it = FindKey(key);
  return (it != sg_keyInstances.end()) ? it->second : InputInvalid;
}

KInputManager::InputState KInputManager::buttonState(Qt::MouseButton button)
{
  ButtonContainer::iterator it = FindButton(button);
  return (it != sg_buttonInstances.end()) ? it->second : InputInvalid;
}

KPoint KInputManager::mousePosition()
{
  return QCursor::pos();
}

KPoint KInputManager::mouseDelta()
{
  return sg_mouseDelta;
}

void KInputManager::update()
{
  // Update Mouse Delta
  sg_mousePrevPosition = sg_mouseCurrPosition;
  sg_mouseCurrPosition = QCursor::pos();
  sg_mouseDelta = sg_mouseCurrPosition - sg_mousePrevPosition;

  // Update KeyState values
  Update(sg_buttonInstances);
  Update(sg_keyInstances);
}

void KInputManager::registerKeyPress(int key)
{
  KeyContainer::iterator it = FindKey((Qt::Key)key);
  if (it == sg_keyInstances.end())
  {
    sg_keyInstances.push_back(KeyInstance((Qt::Key)key, InputRegistered));
  }
}

void KInputManager::registerKeyRelease(int key)
{
  KeyContainer::iterator it = FindKey((Qt::Key)key);
  if (it != sg_keyInstances.end())
  {
    it->second = InputUnregistered;
  }
}

void KInputManager::registerMousePress(Qt::MouseButton button)
{
  ButtonContainer::iterator it = FindButton(button);
  if (it == sg_buttonInstances.end())
  {
    sg_buttonInstances.push_back(ButtonInstance(button, InputRegistered));
  }
}

void KInputManager::registerMouseRelease(Qt::MouseButton button)
{
  ButtonContainer::iterator it = FindButton(button);
  if (it != sg_buttonInstances.end())
  {
    it->second = InputUnregistered;
  }
}

void KInputManager::reset()
{
  sg_keyInstances.clear();
  sg_buttonInstances.clear();
}
