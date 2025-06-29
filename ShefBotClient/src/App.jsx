import React, { useState, useEffect, useRef } from 'react';

const SmartPotApp = () => 
  {
  const [currentPage, setCurrentPage] = useState('home');
  const [timerMinutes, setTimerMinutes] = useState(0);
  const [riceCups, setRiceCups] = useState(1);
  const [isTimerRunning, setIsTimerRunning] = useState(false);
  const [isCookingStarted, setIsCookingStarted] = useState(false);
  const [currentTimer, setCurrentTimer] = useState(0);
  const [isTimerFinished, setIsTimerFinished] = useState(false);
  const [riceStatus, setRiceStatus] = useState('idle'); 
  const [statusData, setStatusData] = useState({ temp: 0, water: 0, fire: "off" });

  const [isLoading, setIsLoading] = useState(false);
  const [connectionError, setConnectionError] = useState('');

  const ESP32_IP = "192.168.0.101";
  useEffect(() => {
    const interval = setInterval(async () => {
      try {
        const res = await fetch(`http://${ESP32_IP}/status`);
        const data = await res.json();
        setStatusData(data);
      } catch (err) {
        console.error("שגיאה בקריאת סטטוס:", err);
      }
    }, 5000); // מתעדכן כל 5 שניות
  
    return () => clearInterval(interval);
  }, []);
  useEffect(() => {
    let interval;
    if (isTimerRunning && currentTimer > 0) {
      interval = setInterval(() => {
        setCurrentTimer(prev => {
          if (prev <= 1) {
            setIsTimerRunning(false);
            setIsTimerFinished(true);
            return 0;
          }
          return prev - 1;
        });
      }, 1000);
    }
    
    return () => {
      if (interval) {
        clearInterval(interval);
      }
    };
  }, [isTimerRunning, currentTimer]);

  useEffect(() => {
    let interval;
    if (riceStatus !== 'idle' && riceStatus !== 'completed') {
      interval = setInterval(async () => {
        try {
          const response = await fetch(`http://${ESP32_IP}/rice-status`);
          const data = await response.json();
          setRiceStatus(data.status);
        } catch (error) {
          console.log('Error checking rice status:', error);
        }
      }, 3000);
    }
    
    return () => {
      if (interval) clearInterval(interval);
    };
  }, [riceStatus]);
  
  const calculateRecipe = (cups) => {
    return {
      oil: cups * 2,
      rice: cups,
      salt: cups,
      water: cups * 1.5
    };
  };

  const startTimerCooking = async (minutes) => {
    setIsLoading(true);
    setConnectionError('');
  
    try {
      console.log(`שולח טיימר ל-ESP32: ${minutes} דקות`);
  
      const response = await fetch(`http://${ESP32_IP}/start-timer`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ minutes }),
        signal: AbortSignal.timeout(10000)
      });

      if (!response.ok) {
        throw new Error(`שגיאת שרת: ${response.status} - ${response.statusText}`);
      }
  
      const message = await response.text();
      console.log("התגובה מה-ESP32:", message);
      setIsTimerRunning(true);
      setCurrentTimer(minutes * 60);
    } catch (error) {
      console.error("שגיאה בשליחת טיימר ל-ESP32:", error);
      if (error.name === 'AbortError') {
        setConnectionError('תקלה: החיבור לסיר לוקח יותר מדי זמן');
      } else if (error.message.includes('CORS') || error.name === 'TypeError') {
        setConnectionError('תקלה: לא ניתן להתחבר לסיר. בדק את החיבור לWiFi וש-ESP32 פועל');
      } else {
        setConnectionError(` ${error.message}`);
      }
    } finally {
      setIsLoading(false);
    }
  };
  
  const startRiceCooking = async (cups) => {
    setIsLoading(true);
    setConnectionError('');
    
    try {
      console.log(`שולח בקשה לESP32: ${cups} כוסות`);
      
      const response = await fetch(`http://${ESP32_IP}/start-rice`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ cups }),
        signal: AbortSignal.timeout(30000)
      });
  
      if (!response.ok) {
        throw new Error(`השרת החזיר שגיאה: ${response.status} - ${response.statusText}`);
      }

      const message = await response.text();
      console.log("תשובה מהESP32:", message);
      
      setRiceStatus('cooking');
      
      setTimeout(() => {
        setCurrentPage('rice');
      }, 3000);

    } catch (error) {
      console.error("שגיאה בחיבור לESP32:", error);
      
      if (error.name === 'AbortError') {
        setConnectionError('תקלה: החיבור לסיר לוקח יותר מדי זמן');
      } else if (error.message.includes('CORS') || error.name === 'TypeError') {
        setConnectionError('תקלה: לא ניתן להתחבר לסיר. בדק את החיבור לWiFi וש-ESP32 פועל');
      } else {
        setConnectionError(`תקלה: ${error.message}`);
      }
    } finally {
      setIsLoading(false);
    }
  };

  const checkConnection = async () => {
    try {
      const response = await fetch(`http://${ESP32_IP}/status`, {
        method: "GET",
        signal: AbortSignal.timeout(5000)
      });
      
      if (response.ok) {
        console.log("החיבור לסיר תקין");
        return true;
      }
    } catch (error) {
      console.log("אין חיבור לסיר");
      return false;
    }
  };

  const recipe = calculateRecipe(riceCups);

  const styles = {
    app: {
      minHeight: '100vh',
      background: 'linear-gradient(135deg, #fb923c 0%, #ef4444 50%, #ec4899 100%)',
      fontFamily: 'system-ui, -apple-system, sans-serif',
      color: 'white',
      margin: 0,
      padding: 0
    },
    container: {
      minHeight: '100vh',
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      justifyContent: 'center',
      padding: '32px',
      textAlign: 'center'
    },
    logo: {
      position: 'relative',
      marginBottom: '32px'
    },
    logoCircle: {
      width: '160px',
      height: '160px',
      margin: '0 auto',
      backgroundColor: 'white',
      borderRadius: '50%',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      boxShadow: '0 25px 50px rgba(0,0,0,0.3)',
      transform: 'rotate(3deg)',
      transition: 'transform 0.3s ease',
      cursor: 'pointer'
    },
    logoIcon: {
      fontSize: '80px',
      position: 'relative'
    },
    logoDecor1: {
      position: 'absolute',
      top: '-8px',
      right: '-8px',
      width: '32px',
      height: '32px',
      backgroundColor: '#ef4444',
      borderRadius: '50%',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      fontSize: '16px'
    },
    logoDecor2: {
      position: 'absolute',
      bottom: '-4px',
      left: '-4px',
      width: '24px',
      height: '24px',
      backgroundColor: '#fbbf24',
      borderRadius: '50%',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      fontSize: '12px'
    },
    steamEffect: {
      position: 'absolute',
      top: '0',
      left: '50%',
      transform: 'translateX(-50%) translateY(-16px)',
      display: 'flex',
      gap: '4px'
    },
    steamBubble: {
      width: '8px',
      height: '24px',
      backgroundColor: 'rgba(255,255,255,0.7)',
      borderRadius: '4px'
    },
    title: {
      fontSize: '64px',
      fontWeight: 'bold',
      background: 'linear-gradient(to right, #fef3c7, white)',
      WebkitBackgroundClip: 'text',
      backgroundClip: 'text',
      color: 'transparent',
      marginBottom: '16px'
    },
    subtitle: {
      fontSize: '20px',
      color: 'rgba(255,255,255,0.9)',
      fontWeight: '500',
      marginBottom: '8px'
    },
    description: {
      fontSize: '18px',
      color: 'rgba(255,255,255,0.8)',
      marginBottom: '48px'
    },
    button: {
      backgroundColor: 'white',
      color: '#ea580c',
      padding: '16px 48px',
      borderRadius: '50px',
      fontSize: '20px',
      fontWeight: 'bold',
      border: 'none',
      cursor: 'pointer',
      boxShadow: '0 25px 50px rgba(0,0,0,0.3)',
      transition: 'all 0.3s ease',
      display: 'inline-flex',
      alignItems: 'center',
      gap: '12px'
    },
    menuTitle: {
      fontSize: '48px',
      fontWeight: 'bold',
      marginBottom: '16px'
    },
    bonAppetit: {
      fontSize: '24px',
      color: 'rgba(255,255,255,0.9)',
      fontStyle: 'italic',
      marginBottom: '48px'
    },
    menuGrid: {
      display: 'grid',
      gridTemplateColumns: 'repeat(auto-fit, minmax(300px, 1fr))',
      gap: '32px',
      width: '100%',
      maxWidth: '800px',
      marginBottom: '48px'
    },
    menuButton: {
      background: 'linear-gradient(135deg, #fb923c 0%, #fbbf24 100%)',
      padding: '32px',
      borderRadius: '24px',
      border: 'none',
      cursor: 'pointer',
      boxShadow: '0 25px 50px rgba(0,0,0,0.3)',
      transition: 'all 0.3s ease',
      color: 'white'
    },
    menuButtonTimer: {
      background: 'linear-gradient(135deg, #ec4899 0%, #ef4444 100%)'
    },
    menuButtonRecipes: {
      background: 'linear-gradient(135deg, #fbbf24 0%, #fb923c 100%)'
    },
    menuButtonOnion: {
      background: 'linear-gradient(135deg, #ef4444 0%, #ec4899 100%)'
    },
    menuIcon: {
      fontSize: '64px',
      marginBottom: '16px',
      transition: 'transform 0.3s ease',
      display: 'block'
    },
    menuButtonTitle: {
      fontSize: '24px',
      fontWeight: 'bold',
      marginBottom: '8px'
    },
    menuButtonDesc: {
      fontSize: '16px',
      opacity: 0.9
    },
    backButton: {
      backgroundColor: 'rgba(255,255,255,0.2)',
      color: 'white',
      padding: '12px 24px',
      borderRadius: '50px',
      border: 'none',
      cursor: 'pointer',
      transition: 'all 0.3s ease',
      display: 'inline-flex',
      alignItems: 'center',
      gap: '8px'
    },
    timerSection: {
      backgroundColor: 'rgba(255,255,255,0.2)',
      padding: '32px',
      borderRadius: '24px',
      marginBottom: '32px'
    },
    timerLabel: {
      fontSize: '20px',
      marginBottom: '24px'
    },
    timerControls: {
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      gap: '24px'
    },
    timerButton: {
      width: '48px',
      height: '48px',
      backgroundColor: 'rgba(255,255,255,0.3)',
      borderRadius: '50%',
      border: 'none',
      cursor: 'pointer',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center',
      fontSize: '24px',
      color: 'white',
      transition: 'all 0.3s ease'
    },
    timerDisplay: {
      fontSize: '64px',
      fontWeight: 'bold',
      backgroundColor: 'rgba(255,255,255,0.2)',
      padding: '16px 32px',
      borderRadius: '16px',
      minWidth: '120px'
    },
    startButton: {
      background: 'linear-gradient(135deg, #fbbf24, white)',
      color: '#dc2626',
      padding: '16px 32px',
      borderRadius: '50px',
      fontSize: '20px',
      fontWeight: 'bold',
      border: 'none',
      cursor: 'pointer',
      boxShadow: '0 25px 50px rgba(0,0,0,0.3)',
      transition: 'all 0.3s ease',
      display: 'inline-flex',
      alignItems: 'center',
      gap: '12px'
    },
    completionMessage: {
      fontSize: '32px',
      fontWeight: 'bold',
      backgroundColor: '#22c55e',
      padding: '32px',
      borderRadius: '24px'
    },
    recipeSection: {
      backgroundColor: 'rgba(255,255,255,0.2)',
      padding: '32px',
      borderRadius: '24px',
      marginBottom: '32px',
      textAlign: 'right',
      maxWidth: '600px'
    },
    recipeTitle: {
      fontSize: '24px',
      fontWeight: 'bold',
      marginBottom: '24px',
      textAlign: 'center'
    },
    recipeItem: {
      display: 'flex',
      justifyContent: 'space-between',
      alignItems: 'center',
      backgroundColor: 'rgba(255,255,255,0.1)',
      padding: '12px',
      borderRadius: '8px',
      marginBottom: '12px',
      fontSize: '18px'
    },
    errorMessage: {
      backgroundColor: '#ef4444',
      color: 'white',
      padding: '16px',
      borderRadius: '12px',
      marginTop: '16px',
      fontSize: '16px',
      textAlign: 'center'
    },
    errorButton: {
      backgroundColor: 'transparent',
      border: '2px solid white',
      color: 'white',
      padding: '8px 16px',
      borderRadius: '8px',
      marginTop: '8px',
      cursor: 'pointer',
      display: 'block',
      margin: '8px auto 0'
    },
    decorCircle1: {
      position: 'absolute',
      top: '80px',
      left: '80px',
      width: '64px',
      height: '64px',
      backgroundColor: 'rgba(255,193,7,0.2)',
      borderRadius: '50%'
    },
    decorCircle2: {
      position: 'absolute',
      bottom: '80px',
      right: '80px',
      width: '48px',
      height: '48px',
      backgroundColor: 'rgba(236,72,153,0.3)',
      borderRadius: '50%'
    },
    decorCircle3: {
      position: 'absolute',
      top: '33%',
      right: '40px',
      width: '32px',
      height: '32px',
      backgroundColor: 'rgba(251,146,60,0.25)',
      borderRadius: '50%'
    }
  };

  const HomePage = () => (
    <div style={styles.app}>
      <div style={styles.container}>
        <div style={styles.logo}>
          <div style={{...styles.logoCircle}} 
              onMouseEnter={e => e.target.style.transform = 'rotate(6deg)'}
              onMouseLeave={e => e.target.style.transform = 'rotate(3deg)'}>
            <div style={styles.logoIcon}>
              👨‍🍳
              <div style={styles.logoDecor1}>⏰</div>
              <div style={styles.logoDecor2}>🍽️</div>
            </div>
          </div>
          <div style={styles.steamEffect}>
            <div style={{...styles.steamBubble, animationDelay: '0s'}}></div>
            <div style={{...styles.steamBubble, animationDelay: '0.3s'}}></div>
            <div style={{...styles.steamBubble, animationDelay: '0.6s'}}></div>
          </div>
        </div>

        <div>
          <h1 style={styles.title}>ShefBot</h1>
          <p style={styles.subtitle}>מערכת בישול חכמה ואוטומטית</p>
          <p style={styles.description}>בישול מדויק, הנחיות קוליות וניטור אוטומטי בזמן אמת</p>
        </div>

        <button 
          style={styles.button}
          onClick={() => setCurrentPage('menu')}
          onMouseEnter={e => e.target.style.transform = 'scale(1.05)'}
          onMouseLeave={e => e.target.style.transform = 'scale(1)'}
        >
          התחל לבשל
          <span>→</span>
        </button>

        <div style={styles.decorCircle1}></div>
        <div style={styles.decorCircle2}></div>
        <div style={styles.decorCircle3}></div>
      </div>
    </div>
  );

  const MenuPage = () => {
    const timerIconRef = useRef(null);
    const recipesIconRef = useRef(null);

    return (
      <div style={styles.app}>
        <div style={styles.container}>
          <div>
            <h1 style={styles.menuTitle}>בחר מצב בישול</h1>
            <p style={styles.bonAppetit}>Bon Appétit!</p>
          </div>

          <div style={styles.menuGrid}>
            <button 
              style={{...styles.menuButton, ...styles.menuButtonRecipes}}
              onClick={() => setCurrentPage('recipes')}
              onMouseEnter={e => {
                e.target.style.transform = 'scale(1.05)';
                if (recipesIconRef.current) {
                  recipesIconRef.current.style.transform = 'rotate(12deg)';
                }
              }}
              onMouseLeave={e => {
                e.target.style.transform = 'scale(1)';
                if (recipesIconRef.current) {
                  recipesIconRef.current.style.transform = 'rotate(0deg)';
                }
              }}
            >
              <div>
                <div ref={recipesIconRef} style={styles.menuIcon}>🍚</div>
                <h3 style={styles.menuButtonTitle}>הכנת ארוחה</h3>
                <p style={styles.menuButtonDesc}>מתכונים מוכנים עם הנחיות</p>
              </div>
            </button>

            <button 
              style={{...styles.menuButton, ...styles.menuButtonTimer}}
              onClick={() => setCurrentPage('timer')}
              onMouseEnter={e => {
                e.target.style.transform = 'scale(1.05)';
                if (timerIconRef.current) {
                  timerIconRef.current.style.transform = 'rotate(360deg)';
                }
              }}
              onMouseLeave={e => {
                e.target.style.transform = 'scale(1)';
                if (timerIconRef.current) {
                  timerIconRef.current.style.transform = 'rotate(0deg)';
                }
              }}
            >
              <div>
                <div ref={timerIconRef} style={styles.menuIcon}>⏰</div>
                <h3 style={styles.menuButtonTitle}>טיימר</h3>
                <p style={styles.menuButtonDesc}>קבע זמן בישול מותאם אישית</p>
              </div>
            </button>
          </div>

          <button 
            style={styles.backButton}
            onClick={() => setCurrentPage('home')}
            onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
            onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.2)'}
          >
            🏠 חזור לבית
          </button>
        </div>
      </div>
    );
  };

  const TimerPage = () => (
    <div style={styles.app}>
      <div style={styles.container}>
        <h1 style={styles.menuTitle}>טיימר בישול</h1>
        
        {!isTimerRunning && !isTimerFinished ? (
          <div>
            <div style={styles.timerSection}>
              <p style={styles.timerLabel}>הגדר זמן בישול (דקות)</p>
              <div style={styles.timerControls}>
                <button 
                  style={styles.timerButton}
                  onClick={() => setTimerMinutes(Math.max(0, timerMinutes - 1))}
                  onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.4)'}
                  onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
                >
                  −
                </button>
                
                <div style={styles.timerDisplay}>
                  {timerMinutes}
                </div>
                
                <button 
                  style={styles.timerButton}
                  onClick={() => setTimerMinutes(timerMinutes + 1)}
                  onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.4)'}
                  onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
                >
                  +
                </button>
              </div>
            </div>

            <button 
              style={{...styles.startButton, opacity: timerMinutes === 0 ? 0.5 : 1, cursor: timerMinutes === 0 ? 'not-allowed' : 'pointer'}}
              onClick={() => {
                if (timerMinutes > 0) {
                  setCurrentTimer(timerMinutes * 60); 
                  setIsTimerRunning(true);
                  setIsTimerFinished(false);
                  startTimerCooking(timerMinutes);    
                }
              }}
              disabled={timerMinutes === 0}
              onMouseEnter={e => timerMinutes > 0 && (e.target.style.transform = 'scale(1.05)')}
              onMouseLeave={e => e.target.style.transform = 'scale(1)'}
            >
              ▶️ התחל בישול
            </button>

            {connectionError && (
              <div style={styles.errorMessage}>
                {connectionError}
                <button 
                  style={styles.errorButton}
                  onClick={() => setConnectionError('')}
                >
                  ❌ סגור
                </button>
              </div>
            )}
          </div>
        ) : isTimerRunning ? (
          <div>
            <div style={styles.timerSection}>
              <p style={styles.timerLabel}>הטיימר פועל...</p>
              <div style={styles.timerDisplay}>
                {Math.floor(currentTimer / 60)}:{String(currentTimer % 60).padStart(2, '0')}
              </div>
            </div>
            <button 
              style={{...styles.backButton, backgroundColor: 'rgba(220, 38, 38, 0.8)', color: 'white'}}
              onClick={() => {
                setIsTimerRunning(false); 
                
                fetch('http://192.168.0.101/stop-timer', {
                  method: 'POST',
                  headers: {
                    'Content-Type': 'application/json'
                  },
                  body: JSON.stringify({ message: 'stop_timer' })
                })
                .then(res => {
                  if (!res.ok) {
                    throw new Error('Failed to send stop command');
                  }
                  return res.text();
                })
                .then(data => {
                  console.log('Server response:', data);
                })
                .catch(err => {
                  console.error('Error sending stop command:', err);
                });
              }}
              onMouseEnter={e => e.target.style.backgroundColor = 'rgba(220, 38, 38, 0.9)'}
              onMouseLeave={e => e.target.style.backgroundColor = 'rgba(220, 38, 38, 0.8)'}
            >
              ⏹️ עצור טיימר
            </button>
          </div>
        ) : (
          <div style={styles.completionMessage}>
            🎉 הבישול הסתיים! 🎉
          </div>
        )}

        <div style={{marginTop: '32px'}}>
          <button 
            style={styles.backButton}
            onClick={() => {
              setCurrentPage('menu');
              setIsTimerRunning(false);
              setTimerMinutes(0);
              setCurrentTimer(0);
              setIsTimerFinished(false);
              setConnectionError('');
            }}
            onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
            onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.2)'}
          >
            🏠 חזור לתפריט
          </button>
        </div>
      </div>
    </div>
  );

  const RecipesPage = () => {
    const riceIconRef = useRef(null);
    const onionIconRef = useRef(null);

    return (
      <div style={styles.app}>
        <div style={styles.container}>
          <h1 style={styles.menuTitle}>בחר מתכון</h1>
          
          <div style={styles.menuGrid}>
            <button 
              style={{...styles.menuButton, ...styles.menuButtonRecipes}}
              onClick={() => setCurrentPage('rice')}
              onMouseEnter={e => {
                e.target.style.transform = 'scale(1.05)';
                if (riceIconRef.current) {
                  riceIconRef.current.style.transform = 'rotate(12deg)';
                }
              }}
              onMouseLeave={e => {
                e.target.style.transform = 'scale(1)';
                if (riceIconRef.current) {
                  riceIconRef.current.style.transform = 'rotate(0deg)';
                }
              }}
            >
              <div>
                <div ref={riceIconRef} style={styles.menuIcon}>🍚</div>
                <h3 style={styles.menuButtonTitle}>אורז</h3>
                <p style={styles.menuButtonDesc}>אורז לבן טעים ואוורירי</p>
              </div>
            </button>

            <button 
            style={{...styles.menuButton, ...styles.menuButtonOnion}}
            onClick={() => {
            alert("מרק בצל יתווסף בקרוב! עקבו אחרינו לעדכונים חמים 🍲");
            }}
            onMouseEnter={e => {
            e.target.style.transform = 'scale(1.05)';
            if (onionIconRef.current) {
              onionIconRef.current.style.transform = 'rotate(12deg)';
            }
            }}
            onMouseLeave={e => {
              e.target.style.transform = 'scale(1)';
              if (onionIconRef.current) {
                onionIconRef.current.style.transform = 'rotate(0deg)';
              }
             }}
              >
              <div>
                <div ref={onionIconRef} style={styles.menuIcon}>🍲</div>
                <h3 style={styles.menuButtonTitle}>מרק בצל</h3>
                <p style={styles.menuButtonDesc}>מרק בצל חם ומפנק</p>
              </div>
          </button>
          </div>

          <button 
            style={styles.backButton}
            onClick={() => setCurrentPage('menu')}
            onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
            onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.2)'}
          >
            🏠 חזור לתפריט
          </button>
        </div>
      </div>
    );
  };

  const RicePage = () => (
    <div style={styles.app}>
      <div style={styles.container}>
        {riceStatus !== 'completed' ? (
          <>
            <h1 style={styles.menuTitle}>הכנת אורז</h1>
            
            <div style={styles.timerSection}>
              <p style={styles.timerLabel}>בחר כמות כוסות (1-4)</p>
              <div style={styles.timerControls}>
                <button 
                  style={styles.timerButton}
                  onClick={() => setRiceCups(Math.max(1, riceCups - 1))}
                  onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.4)'}
                  onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
                >
                  −
                </button>
                
                <div style={styles.timerDisplay}>
                  {riceCups}
                </div>
                
                <button 
                  style={styles.timerButton}
                  onClick={() => setRiceCups(Math.min(4, riceCups + 1))}
                  onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.4)'}
                  onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
                >
                  +
                </button>
                
              </div>
              
            </div>

            <button 
              style={{...styles.button, backgroundColor: 'white', color: '#ea580c'}}
              onClick={() => setCurrentPage('rice-recipe')}
              onMouseEnter={e => e.target.style.transform = 'scale(1.05)'}
              onMouseLeave={e => e.target.style.transform = 'scale(1)'}
            >
              ENTER
            </button>
            <div style={{ marginTop: "48px", background: "rgba(255,255,255,0.1)", padding: "16px", borderRadius: "12px", width: "100%", maxWidth: "400px" }}>
            <h3>סטטוס בזמן אמת:</h3>
            <p>🌡️ טמפרטורה: {statusData.temp.toFixed(1)}</p>
            <p>💧 מים: {statusData.water} / 1000</p>
      </div>
          </>
          
       ) : null}

       {riceStatus === 'completed' && (
         <div style={styles.completionMessage}>
           🍚 האורז מוכן! בתיאבון! 🍚
         </div>
         
       )}

        <div style={{marginTop: '32px'}}>
          <button 
            style={styles.backButton}
            onClick={() => {
              setCurrentPage('recipes');
              setRiceStatus('idle');
              setRiceCups(1);
              setConnectionError('');
            }}
            onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
            onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.2)'}
          >
            🏠 חזור למתכונים
          </button>
        </div>
      </div>
    </div>
  );

  const RiceRecipePage = () => (
    <div style={styles.app}>
      <div style={styles.container}>
        <h1 style={{...styles.menuTitle, fontSize: '36px'}}>מתכון לאורז - {riceCups} כוסות</h1>
        
        <div style={styles.recipeSection}>
          <h3 style={styles.recipeTitle}>רשימת רכיבים:</h3>
          <div>
            <div style={styles.recipeItem}>
              <span>{recipe.oil} כפות שמן זית</span>
              <span>🫒</span>
            </div>
            <div style={styles.recipeItem}>
              <span>{riceCups} כוס{riceCups > 1 ? 'ות' : ''} אורז</span>
              <span>🍚</span>
            </div>
            <div style={styles.recipeItem}>
              <span>{recipe.salt} {riceCups > 1 ? 'כפיות' : 'כפית'} מלח</span>
              <span>🧂</span>
            </div>
            <div style={styles.recipeItem}>
              <span>{recipe.water} כוס{recipe.water > 1 ? 'ות' : ''} מים רותחים</span>
              <span>💧</span>
            </div>
          </div>
        </div>

        <button 
          style={{
            ...styles.startButton,
            opacity: isLoading ? 0.7 : 1,
            cursor: isLoading ? 'not-allowed' : 'pointer'
          }}
          onClick={() => startRiceCooking(riceCups)}
          disabled={isLoading}
          onMouseEnter={e => !isLoading && (e.target.style.transform = 'scale(1.05)')}
          onMouseLeave={e => e.target.style.transform = 'scale(1)'}
        >
          {isLoading ? (
            <>⏳ שולח פקודה לסיר...</>
          ) : (
            <>▶️ START COOKING</>
          )}
        </button>

        {connectionError && (
          <div style={styles.errorMessage}>
            {connectionError}
            <button 
              style={styles.errorButton}
              onClick={() => setConnectionError('')}
            >
              ❌ סגור
            </button>
          </div>
        )}

        <div style={{marginTop: '32px'}}>
          <button 
            style={styles.backButton}
            onClick={() => {
              setCurrentPage('rice');
              setConnectionError(''); 
            }}
            onMouseEnter={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.3)'}
            onMouseLeave={e => e.target.style.backgroundColor = 'rgba(255,255,255,0.2)'}
          >
            🏠 חזור
          </button>
        </div>
      </div>
    </div>
  );

  const renderCurrentPage = () => {
    switch(currentPage) {
      case 'home': return <HomePage />;
      case 'menu': return <MenuPage />;
      case 'timer': return <TimerPage />;
      case 'recipes': return <RecipesPage />;
      case 'rice': return <RicePage />;
      case 'rice-recipe': return <RiceRecipePage />;
      default: return <HomePage />;
    }
  };

  return renderCurrentPage();
};

export default SmartPotApp;