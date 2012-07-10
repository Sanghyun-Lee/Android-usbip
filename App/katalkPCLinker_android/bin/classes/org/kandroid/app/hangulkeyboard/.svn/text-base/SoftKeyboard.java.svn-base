/*
 * Copyright (C) 2008-2009 Google Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 * 
 * Some Hangul InputMethod Code added by www.kandroid.org
 * 
 */

package org.kandroid.app.hangulkeyboard;

import android.content.Intent;
import android.graphics.Rect;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.net.Uri;
import android.text.method.MetaKeyKeyListener;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.net.URI;
import java.util.ArrayList;
import java.util.List;


/**
 * Example of writing an input method for a soft keyboard.  This code is
 * focused on simplicity over completeness, so it should in no way be considered
 * to be a complete soft keyboard implementation.  Its purpose is to provide
 * a basic example for how you would get started writing an input method, to
 * be fleshed out as appropriate.
 */
public class SoftKeyboard extends InputMethodService 
        implements KeyboardView.OnKeyboardActionListener {
    static final boolean DEBUG = false;
    
    /**
     * This boolean indicates the optional example code for performing
     * processing of hard keys in addition to regular text generation
     * from on-screen interaction.  It would be used for input methods that
     * perform language translations (such as converting text entered on 
     * a QWERTY keyboard to Chinese), but may not be used for input methods
     * that are primarily intended to be used for on-screen text entry.
     */
    static final boolean PROCESS_HARD_KEYS = true;
    
    private KeyboardView mInputView;
    private CandidateView mCandidateView;
    private CompletionInfo[] mCompletions;
    
    private StringBuilder mComposing = new StringBuilder();
    private boolean mPredictionOn;
    private boolean mCompletionOn;
    private int mLastDisplayWidth;
    private boolean mCapsLock;
    private long mLastShiftTime;
    private long mMetaState;
    
    private LatinKeyboard mSymbolsKeyboard;
    private LatinKeyboard mSymbolsShiftedKeyboard;
    private LatinKeyboard mQwertyKeyboard;
    
    private Keyboard mHangulKeyboard; // Hangul Code
    private Keyboard mHangulShiftedKeyboard; // Hangul Code
    private Keyboard mSejongKeyboard; 
    
    private LatinKeyboard mCurKeyboard;
    
    private String mWordSeparators;
    
    /**
     * Main initialization of the input method component.  Be sure to call
     * to super class.
     */
    @Override public void onCreate() {
        super.onCreate();
        mWordSeparators = getResources().getString(R.string.word_separators);
    }
    
    /**
     * This is the point where you can do all of your UI initialization.  It
     * is called after creation and any configuration change.
     */
    @Override public void onInitializeInterface() {
        if (mQwertyKeyboard != null) {
            // Configuration changes can happen after the keyboard gets recreated,
            // so we need to be able to re-build the keyboards if the available
            // space has changed.
            int displayWidth = getMaxWidth();
            if (displayWidth == mLastDisplayWidth) return;
            mLastDisplayWidth = displayWidth;
        }
        mQwertyKeyboard = new LatinKeyboard(this, R.xml.qwerty);
        mSymbolsKeyboard = new LatinKeyboard(this, R.xml.symbols);
        mSymbolsShiftedKeyboard = new LatinKeyboard(this, R.xml.symbols_shift);
        mHangulKeyboard = new HangulKeyboard(this, R.xml.hangul);        
        mHangulShiftedKeyboard = new HangulKeyboard(this, R.xml.hangul_shift);
        mSejongKeyboard = new SejongKeyboard(this, R.xml.sejong);
    }
    
    /**
     * Called by the framework when your view for creating input needs to
     * be generated.  This will be called the first time your input method
     * is displayed, and every time it needs to be re-created such as due to
     * a configuration change.
     */
    @Override public View onCreateInputView() {
        mInputView = (KeyboardView) getLayoutInflater().inflate(
                R.layout.input, null);
        mInputView.setOnKeyboardActionListener(this);
        mInputView.setKeyboard(mQwertyKeyboard);
        return mInputView;
    }

    /**
     * Called by the framework when your view for showing candidates needs to
     * be generated, like {@link #onCreateInputView}.
     */
    
    @Override public View onCreateCandidatesView() {
        mCandidateView = new CandidateView(this);
        mCandidateView.setService(this);
        return mCandidateView;
    }
	
    /**
     * This is the main point where we do our initialization of the input method
     * to begin operating on an application.  At this point we have been
     * bound to the client, and are now receiving all of the detailed information
     * about the target of our edits.
     */
    @Override public void onStartInput(EditorInfo attribute, boolean restarting) {
        super.onStartInput(attribute, restarting);
/*        
        Log.i("Hangul", "onStartInput");
*/
        clearHangul();
        clearSejong();
        previousCurPos = -1;        
        
        // Reset our state.  We want to do this even if restarting, because
        // the underlying state of the text editor could have changed in any way.
        mComposing.setLength(0);
        updateCandidates();
        
        if (!restarting) {
            // Clear shift states.
            mMetaState = 0;
        }
        
        mPredictionOn = false;
        mCompletionOn = false;
        mCompletions = null;
        
        // We are now going to initialize our state based on the type of
        // text being edited.
        switch (attribute.inputType&EditorInfo.TYPE_MASK_CLASS) {
            case EditorInfo.TYPE_CLASS_NUMBER:
            case EditorInfo.TYPE_CLASS_DATETIME:
                // Numbers and dates default to the symbols keyboard, with
                // no extra features.
                mCurKeyboard = mSymbolsKeyboard;
                break;
                
            case EditorInfo.TYPE_CLASS_PHONE:
                // Phones will also default to the symbols keyboard, though
                // often you will want to have a dedicated phone keyboard.
                mCurKeyboard = mSymbolsKeyboard;
                break;
                
            case EditorInfo.TYPE_CLASS_TEXT:
                // This is general text editing.  We will default to the
                // normal alphabetic keyboard, and assume that we should
                // be doing predictive text (showing candidates as the
                // user types).
                mCurKeyboard = mQwertyKeyboard;
                mPredictionOn = true;
                
                // We now look for a few special variations of text that will
                // modify our behavior.
                int variation = attribute.inputType &  EditorInfo.TYPE_MASK_VARIATION;
                if (variation == EditorInfo.TYPE_TEXT_VARIATION_PASSWORD ||
                        variation == EditorInfo.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD) {
                    // Do not display predictions / what the user is typing
                    // when they are entering a password.
                    mPredictionOn = false;
                }
                
                if (variation == EditorInfo.TYPE_TEXT_VARIATION_EMAIL_ADDRESS 
                        || variation == EditorInfo.TYPE_TEXT_VARIATION_URI
                        || variation == EditorInfo.TYPE_TEXT_VARIATION_FILTER) {
                    // Our predictions are not useful for e-mail addresses
                    // or URIs.
                    mPredictionOn = false;
                }
                
                if ((attribute.inputType&EditorInfo.TYPE_TEXT_FLAG_AUTO_COMPLETE) != 0) {
                    // If this is an auto-complete text view, then our predictions
                    // will not be shown and instead we will allow the editor
                    // to supply their own.  We only show the editor's
                    // candidates when in fullscreen mode, otherwise relying
                    // own it displaying its own UI.
                    mPredictionOn = false;
                    mCompletionOn = isFullscreenMode();
                }
                
                // We also want to look at the current state of the editor
                // to decide whether our alphabetic keyboard should start out
                // shifted.
                updateShiftKeyState(attribute);
                break;
                
            default:
                // For all unknown input types, default to the alphabetic
                // keyboard with no special features.
                mCurKeyboard = mQwertyKeyboard;
                updateShiftKeyState(attribute);
        }
        
        // Update the label on the enter key, depending on what the application
        // says it will do.
        mCurKeyboard.setImeOptions(getResources(), attribute.imeOptions);
        
    }

    /**
     * This is called when the user is done editing a field.  We can use
     * this to reset our state.
     */
    @Override public void onFinishInput() {
        super.onFinishInput();
        
        // Clear current composing text and candidates.
        mComposing.setLength(0);
        updateCandidates();
        
        // We only hide the candidates window when finishing input on
        // a particular editor, to avoid popping the underlying application
        // up and down if the user is entering text into the bottom of
        // its window.
        setCandidatesViewShown(false);
        
        mCurKeyboard = mQwertyKeyboard;
        if (mInputView != null) {
            mInputView.closing();
        }
    }
    
    @Override public void onStartInputView(EditorInfo attribute, boolean restarting) {
        super.onStartInputView(attribute, restarting);
        // Apply the selected keyboard to the input view.
        mInputView.setKeyboard(mCurKeyboard);
        mInputView.closing();
    }
    
    
    /**
     * Deal with the editor reporting movement of its cursor.
     */
    
    @Override public void onUpdateSelection(int oldSelStart, int oldSelEnd,
            int newSelStart, int newSelEnd,
            int candidatesStart, int candidatesEnd) {
    	
        super.onUpdateSelection(oldSelStart, oldSelEnd, newSelStart, newSelEnd,
                candidatesStart, candidatesEnd);
        
        Log.i("Hangul", "onUpdateSelection :"
        		+ Integer.toString(oldSelStart) + ":"
        		+ Integer.toString(oldSelEnd) + ":"
        		+ Integer.toString(newSelStart) + ":"
        		+ Integer.toString(newSelEnd) + ":"
        		+ Integer.toString(candidatesStart) + ":"
        		+ Integer.toString(candidatesEnd)
        		);
        
        // If the current selection in the text view changes, we should
        // clear whatever candidate text we have.
        Keyboard current = mInputView.getKeyboard();
        if (current == mSejongKeyboard) {
        	
        }
        else if (current != mHangulKeyboard && current != mHangulShiftedKeyboard) {        	
	        if (mComposing.length() > 0 && (newSelStart != candidatesEnd
	                || newSelEnd != candidatesEnd)) {
	            mComposing.setLength(0);
	            updateCandidates();
	            InputConnection ic = getCurrentInputConnection();
	            if (ic != null) {
	                ic.finishComposingText();
	            }
	        }
        }
        else {
        	if (mComposing.length() > 0 && (newSelStart != candidatesEnd
	                || newSelEnd != candidatesEnd)) {
	            mComposing.setLength(0);
//	            updateCandidates();
	            clearHangul();
	            InputConnection ic = getCurrentInputConnection();
	            if (ic != null) {
	                ic.finishComposingText();
	            }        		
        	}
        }        
    }

    /**
     * This tells us about completions that the editor has determined based
     * on the current text in it.  We want to use this in fullscreen mode
     * to show the completions ourself, since the editor can not be seen
     * in that situation.
     */
    @Override public void onDisplayCompletions(CompletionInfo[] completions) {
        if (mCompletionOn) {
            mCompletions = completions;
            if (completions == null) {
                setSuggestions(null, false, false);
                return;
            }
            
            List<String> stringList = new ArrayList<String>();
            for (int i=0; i<(completions != null ? completions.length : 0); i++) {
                CompletionInfo ci = completions[i];
                if (ci != null) stringList.add(ci.getText().toString());
            }
            setSuggestions(stringList, true, true);
        }
    }
    
    /**
     * This translates incoming hard key events in to edit operations on an
     * InputConnection.  It is only needed when using the
     * PROCESS_HARD_KEYS option.
     */
    private boolean translateKeyDown(int keyCode, KeyEvent event) {
        mMetaState = MetaKeyKeyListener.handleKeyDown(mMetaState,
                keyCode, event);
        int c = event.getUnicodeChar(MetaKeyKeyListener.getMetaState(mMetaState));
        mMetaState = MetaKeyKeyListener.adjustMetaAfterKeypress(mMetaState);
        InputConnection ic = getCurrentInputConnection();
        if (c == 0 || ic == null) {
            return false;
        }
        
        boolean dead = false;

        if ((c & KeyCharacterMap.COMBINING_ACCENT) != 0) {
            dead = true;
            c = c & KeyCharacterMap.COMBINING_ACCENT_MASK;
        }
        
        if (mComposing.length() > 0) {
            char accent = mComposing.charAt(mComposing.length() -1 );
            int composed = KeyEvent.getDeadChar(accent, c);

            if (composed != 0) {
                c = composed;
                mComposing.setLength(mComposing.length()-1);
            }
        }
        
        onKey(c, null);
        
        return true;
    }
    
    /**
     * Use this to monitor key events being delivered to the application.
     * We get first crack at them, and can either resume them or let them
     * continue to the app.
     */
    @Override public boolean onKeyDown(int keyCode, KeyEvent event) {
    	Log.i("Hangul", "onKeyDown :" + Integer.toString(keyCode));

        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                // The InputMethodService already takes care of the back
                // key for us, to dismiss the input method if it is shown.
                // However, our keyboard could be showing a pop-up window
                // that back should dismiss, so we first allow it to do that.
                if (event.getRepeatCount() == 0 && mInputView != null) {
                    if (mInputView.handleBack()) {
                        return true;
                    }
                } 
                break;
                
            case KeyEvent.KEYCODE_DEL:
                // Special handling of the delete key: if we currently are
                // composing text for the user, we want to modify that instead
                // of let the application to the delete itself.
                if (mComposing.length() > 0) {
                    onKey(Keyboard.KEYCODE_DELETE, null);
                    return true;
                }
                break;
                
            case KeyEvent.KEYCODE_ENTER:
                // Let the underlying text editor always handle these.
                return false;
                
            default:
                // For all other keys, if we want to do transformations on
                // text being entered with a hard keyboard, we need to process
                // it and do the appropriate action.
                if (PROCESS_HARD_KEYS) {
                    if (keyCode == KeyEvent.KEYCODE_SPACE
                            && (event.getMetaState()&KeyEvent.META_ALT_ON) != 0) {
                        // A silly example: in our input method, Alt+Space
                        // is a shortcut for 'android' in lower case.
                        InputConnection ic = getCurrentInputConnection();
                        if (ic != null) {
                            // First, tell the editor that it is no longer in the
                            // shift state, since we are consuming this.
                            ic.clearMetaKeyStates(KeyEvent.META_ALT_ON);
                            keyDownUp(KeyEvent.KEYCODE_A);
                            keyDownUp(KeyEvent.KEYCODE_N);
                            keyDownUp(KeyEvent.KEYCODE_D);
                            keyDownUp(KeyEvent.KEYCODE_R);
                            keyDownUp(KeyEvent.KEYCODE_O);
                            keyDownUp(KeyEvent.KEYCODE_I);
                            keyDownUp(KeyEvent.KEYCODE_D);
                            // And we consume this event.
                            return true;
                        }
                    }
                    if (mPredictionOn && translateKeyDown(keyCode, event)) {
                        return true;
                    }
                }
        }
        
        return super.onKeyDown(keyCode, event);
    }

    /**
     * Use this to monitor key events being delivered to the application.
     * We get first crack at them, and can either resume them or let them
     * continue to the app.
     */
    @Override public boolean onKeyUp(int keyCode, KeyEvent event) {
        // If we want to do transformations on text being entered with a hard
        // keyboard, we need to process the up events to update the meta key
        // state we are tracking.
        if (PROCESS_HARD_KEYS) {
            if (mPredictionOn) {
                mMetaState = MetaKeyKeyListener.handleKeyUp(mMetaState,
                        keyCode, event);
            }
        }
        
        return super.onKeyUp(keyCode, event);
    }

    /**
     * Helper function to commit any text being composed in to the editor.
     */
    private void commitTyped(InputConnection inputConnection) {
        if (mComposing.length() > 0) {
            inputConnection.commitText(mComposing, mComposing.length());
            mComposing.setLength(0);
            updateCandidates();
        }
    }

    /**
     * Helper to update the shift state of our keyboard based on the initial
     * editor state.
     */
    private void updateShiftKeyState(EditorInfo attr) {
        if (attr != null 
                && mInputView != null && mQwertyKeyboard == mInputView.getKeyboard()) {
            int caps = 0;
            EditorInfo ei = getCurrentInputEditorInfo();
            if (ei != null && ei.inputType != EditorInfo.TYPE_NULL) {
                caps = getCurrentInputConnection().getCursorCapsMode(attr.inputType);
            }
            mInputView.setShifted(mCapsLock || caps != 0);
        }
    }
    
    /**
     * Helper to determine if a given character code is alphabetic.
     */
    private boolean isAlphabet(int code) {
        if (Character.isLetter(code)) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     * Helper to send a key down / key up pair to the current editor.
     */
    private void keyDownUp(int keyEventCode) {
        getCurrentInputConnection().sendKeyEvent(
                new KeyEvent(KeyEvent.ACTION_DOWN, keyEventCode));
        getCurrentInputConnection().sendKeyEvent(
                new KeyEvent(KeyEvent.ACTION_UP, keyEventCode));
    }
    
    /**
     * Helper to send a character to the editor as raw key events.
     */
    private void sendKey(int keyCode) {
        switch (keyCode) {
            case '\n':
                keyDownUp(KeyEvent.KEYCODE_ENTER);
                break;
            default:
                if (keyCode >= '0' && keyCode <= '9') {
                    keyDownUp(keyCode - '0' + KeyEvent.KEYCODE_0);
                } else {
                    getCurrentInputConnection().commitText(String.valueOf((char) keyCode), 1);
                }
                break;
        }
    }

    // Implementation of KeyboardViewListener

    public void onKey(int primaryCode, int[] keyCodes) {
        Log.i("Hangul", "onKey PrimaryCode[" + Integer.toString(primaryCode)+"]");
    	
        if (isWordSeparator(primaryCode)) {
            // Handle separator
            Keyboard current = mInputView.getKeyboard();

            if (mComposing.length() > 0) {
                commitTyped(getCurrentInputConnection());
            }

        	if (current == mHangulKeyboard || current == mHangulShiftedKeyboard ) {
                clearHangul();
                sendKey(primaryCode);
            }
            else if (current == mSejongKeyboard) {
            	switch(ko_state_idx) {        	
            	case KO_S_1110:            		
        	        if (mComposing.length() > 0) {
        	            mComposing.setLength(0);
        	        	getCurrentInputConnection().finishComposingText();	        	
        	        }			
                	clearSejong();            	
            		break;
            	default :
                	clearSejong();            	
            		sendKey(primaryCode);
            		break;
            	}
            }
            else {
            	sendKey(primaryCode);
            }
            updateShiftKeyState(getCurrentInputEditorInfo());
        } else if (primaryCode == Keyboard.KEYCODE_DELETE) {
            Keyboard current = mInputView.getKeyboard();
            if (current == mHangulKeyboard || current == mHangulShiftedKeyboard ) {
                hangulSendKey(-2,HCURSOR_NONE);
            }
            else if (current == mSejongKeyboard) {
            	sendSejongKey((char)0,HCURSOR_DELETE);
            }
            else {
                handleBackspace();            	
            }
        } else if (primaryCode == Keyboard.KEYCODE_SHIFT) {
            handleShift();
        } else if (primaryCode == Keyboard.KEYCODE_CANCEL) {
            handleClose();
            return;
        } else if (primaryCode == LatinKeyboardView.KEYCODE_OPTIONS) {
            // Show a menu or somethin'
//        	handleClose();
        	
        	Intent i = new Intent(Intent.ACTION_VIEW);
        	i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        	i.setData(Uri.parse("http://www.kandroid.com/market/product.php?id=1"));
        	startActivity(i);
        	   
        	return;
        } else if (primaryCode == Keyboard.KEYCODE_MODE_CHANGE
                && mInputView != null) {
            Keyboard current = mInputView.getKeyboard();
            if (current == mSymbolsKeyboard || current == mSymbolsShiftedKeyboard) {
                current = mQwertyKeyboard;
            }
            // Hangul Start Code
            else if (current == mQwertyKeyboard) {
                if (mComposing.length() > 0) {
                    commitTyped(getCurrentInputConnection());
                }
                clearHangul();
                current = mHangulKeyboard;
            }            
            // Hangul End Code
            else if (current == mHangulKeyboard) {
            	if (mComposing.length() > 0) {
                  	getCurrentInputConnection().commitText(mComposing, mComposing.length());
    	            mComposing.setLength(0);
            	}            			
            	clearSejong();
            	current = mSejongKeyboard;
            }
            else {
            	if (mComposing.length() > 0) {
                  	getCurrentInputConnection().commitText(mComposing, mComposing.length());
    	            mComposing.setLength(0);
            	}            			
            	clearHangul();            	
                current = mSymbolsKeyboard;
            }
            mInputView.setKeyboard(current);
            if (current == mSymbolsKeyboard) {
                current.setShifted(false);
            }
            
            if (current == mSejongKeyboard) {
            	mInputView.setPreviewEnabled(false);
            }
            else {
            	mInputView.setPreviewEnabled(true);
            }
        } 
        else {
        	/* original code 
            handleCharacter(primaryCode, keyCodes);
            */
        	
        	// Hangul Start Code
        	Keyboard current = mInputView.getKeyboard();
            if (current == mHangulKeyboard || current == mHangulShiftedKeyboard) {
                handleHangul(primaryCode, keyCodes);
            }
            else if (current == mSejongKeyboard) {
            	handleSejong(primaryCode, keyCodes);
            }
            else {
                handleCharacter(primaryCode, keyCodes);
            }        	
            // Hangul End Code
        }
    }

    public void onText(CharSequence text) {
        InputConnection ic = getCurrentInputConnection();
        if (ic == null) return;
        ic.beginBatchEdit();
        if (mComposing.length() > 0) {
            commitTyped(ic);
        }
        ic.commitText(text, 0);
        ic.endBatchEdit();
        updateShiftKeyState(getCurrentInputEditorInfo());
    }

    /**
     * Update the list of available candidates from the current composing
     * text.  This will need to be filled in by however you are determining
     * candidates.
     */
    private void updateCandidates() {
        if (!mCompletionOn) {
            if (mComposing.length() > 0) {
                ArrayList<String> list = new ArrayList<String>();
                list.add(mComposing.toString());
                setSuggestions(list, true, true);
            } else {
                setSuggestions(null, false, false);
            }
        }
    }
    
    public void setSuggestions(List<String> suggestions, boolean completions,
            boolean typedWordValid) {
        if (suggestions != null && suggestions.size() > 0) {
            setCandidatesViewShown(true);
        } else if (isExtractViewShown()) {
            setCandidatesViewShown(true);
        }
        if (mCandidateView != null) {
            mCandidateView.setSuggestions(suggestions, completions, typedWordValid);
        }
    }
    
    private void handleBackspace() {
        final int length = mComposing.length();
        if (length > 1) {
            mComposing.delete(length - 1, length);
            getCurrentInputConnection().setComposingText(mComposing, 1);
            updateCandidates();
        } else if (length > 0) {
            mComposing.setLength(0);
            getCurrentInputConnection().commitText("", 0);
            updateCandidates();
        } else {
            keyDownUp(KeyEvent.KEYCODE_DEL);
        }
        updateShiftKeyState(getCurrentInputEditorInfo());
    }

    private void handleShift() {
        if (mInputView == null) {
            return;
        }
        
        Keyboard currentKeyboard = mInputView.getKeyboard();
        if (mQwertyKeyboard == currentKeyboard) {
            // Alphabet keyboard
            checkToggleCapsLock();
            mInputView.setShifted(mCapsLock || !mInputView.isShifted());
        } 
        // Hangul Code Start
        else if (currentKeyboard == mHangulKeyboard) {
        	mHangulKeyboard.setShifted(true);
            mInputView.setKeyboard(mHangulShiftedKeyboard);
            mHangulShiftedKeyboard.setShifted(true);
            mHangulShiftState = 1;
        } else if (currentKeyboard == mHangulShiftedKeyboard) {
        	mHangulShiftedKeyboard.setShifted(false);
            mInputView.setKeyboard(mHangulKeyboard);
            mHangulKeyboard.setShifted(false);
            mHangulShiftState = 0;
        }
        // Hangul Code End
        else if (currentKeyboard == mSejongKeyboard) {
        	
        }
        else if (currentKeyboard == mSymbolsKeyboard) {
            mSymbolsKeyboard.setShifted(true);
            mInputView.setKeyboard(mSymbolsShiftedKeyboard);
            mSymbolsShiftedKeyboard.setShifted(true);
        } else if (currentKeyboard == mSymbolsShiftedKeyboard) {
            mSymbolsShiftedKeyboard.setShifted(false);
            mInputView.setKeyboard(mSymbolsKeyboard);
            mSymbolsKeyboard.setShifted(false);
        }
    }

// Hangul Code Start
    
    private int isHangulKey(int stack_pos, int new_key) {
    	/*    
        MAP(0,20,1); // ㄱ,ㅅ 
    	MAP(3,23,4); // ㄴ,ㅈ
    	MAP(3,29,5); // ㄴ,ㅎ
    	MAP(8,0,9); // ㄹ,ㄱ
    	MAP(8,16,10); // ㄹ,ㅁ
    	MAP(8,17,11); // ㄹ,ㅂ
    	MAP(8,20,12); // ㄹ,ㅅ
    	MAP(8,27,13); // ㄹ,ㅌ
    	MAP(8,28,14); // ㄹ,ㅍ
    	MAP(8,29,15); // ㄹ,ㅎ
    	MAP(17,20,19); // ㅂ,ㅅ			
    */	    	
        if (stack_pos != 2) {
            switch (mHangulKeyStack[stack_pos]) {
            case 0:
                if (new_key == 20) return 2;
                break;
            case 3:
                if (new_key == 23) return 4;
                else if(new_key == 29) return 5;
                break;
            case 8:
                if (new_key == 0)return 9;
                else if (new_key == 16) return 10;
                else if (new_key == 17) return 11;
                else if (new_key == 20) return 12;
                else if (new_key == 27) return 13;
                else if (new_key == 28) return 14;
                else if (new_key == 29) return 15;
                break;
           case 17:
                if (new_key == 20) return 19;
                break;
           }
        }
        else {
        	/*        	 
     		38, 30, 39 // ㅗ ㅏ ㅘ
    		38, 31, 40 // ㅗ ㅐ ㅙ
    		38, 50, 41 //ㅗ ㅣ ㅚ
    		43, 34, 44 // ㅜ ㅓ ㅝ
    		43, 35, 45 // ㅜ ㅔ ㅞ
    		43, 50, 46 // ㅜ ㅣ ㅟ
    		48, 50, 49 // ㅡ ㅣ ㅢ
*/        	
           switch (mHangulKeyStack[stack_pos]) {
           case 38:
               if (new_key == 30) return 39;
               else if (new_key == 31) return 40;
               else if (new_key == 50) return 41;
               break;
           case 43:
               if (new_key == 34) return 44;
               else if (new_key == 35) return 45;
               else if (new_key == 50) return 46;
               break;
           case 48:
               if (new_key == 50) return 49;
               break;
           }
       }
       return 0;
    }

    private static char HCURSOR_NONE = 0;
    private static char HCURSOR_NEW = 1;
    private static char HCURSOR_ADD = 2;
    private static char HCURSOR_UPDATE = 3;
    private static char HCURSOR_APPEND = 4;
    private static char HCURSOR_UPDATE_LAST = 5;
    private static char HCURSOR_DELETE_LAST = 6;
    private static char HCURSOR_DELETE = 7;
    
    
    private static int mHCursorState = HCURSOR_NONE;
    private static char h_char[] = new char[1];    
    private int previousCurPos = -2;
    private int previousHangulCurPos = -1;
    private int mHangulShiftState = 0;
    private int mHangulState = 0;
    private static int mHangulKeyStack[] = {0,0,0,0,0,0}; // 초,초,중,중,종,종
    private static int mHangulJamoStack[] = {0,0,0};
    final static int H_STATE_0 = 0;
    final static int H_STATE_1 = 1;
    final static int H_STATE_2 = 2;
    final static int H_STATE_3 = 3;
    final static int H_STATE_4 = 4;
    final static int H_STATE_5 = 5;
    final static int H_STATE_6 = 6;
    final static char[] h_chosung_idx =  
    {0,1, 9,2,12,18,3, 4,5, 0, 6,7, 9,16,17,18,6, 7, 8, 9,9,10,11,12,13,14,15,16,17,18};
/*
    {0, 1, 9, 2,12,18, 3,4, 5, 0, 6, 7, 9,16,17, 18,6, 7, 8, 9, 9,10,11, 12, 13,14,15,16,17,18};
//   ㄱ,ㄲ,ㄳ,ㄴ,ㄵ,ㄶ,ㄷ,ㄸ,ㄹ,ㄺ,ㄻ,ㄼ,ㄽ,ㄾ,ㄿ, ㅀ,ㅁ,ㅂ,ㅃ,ㅄ,ㅅ,ㅆ,ㅇ, ㅈ, ㅉ,ㅊ,ㅋ, ㅌ,ㅍ,ㅎ
//   ㄱ,ㄲ,   ㄴ,      ㄷ,ㄸ,ㄹ,                     ㅁ,ㅂ,ㅃ,   ㅅ,ㅆ,ㅇ, ㅈ, ㅉ,ㅊ,ㅋ, ㅌ,ㅍ,ㅎ	
*/    
    final static char[] h_jongsung_idx = 
    {0, 1, 2, 3,4,5, 6, 7, 0,8, 9,10,11,12,13,14,15,16,17,0 ,18,19,20,21,22,0 ,23,24,25,26,27};
/*
    {0, 1, 2, 3, 4, 5, 6, 7, 0,8, 9,10,11, 12,13, 14,15,16, 17,0,18, 19,20,21,22, 0 ,23,24,25,26,27};
//   x, ㄱ,ㄲ,ㄳ,ㄴ,ㄵ,ㄶ,ㄷ,ㄸ,ㄹ,ㄺ,ㄻ,ㄼ, ㄽ,ㄾ, ㄿ,ㅀ,ㅁ, ㅂ,ㅃ,ㅄ, ㅅ,ㅆ, o,ㅈ, ㅉ,ㅊ, ㅋ,ㅌ,ㅍ,ㅎ,
	//  x  ㄱ  ㄲ  ㄳ  ㄴ  ㄵ  ㄶ  ㄷ       ㄹ  ㄺ  ㄻ  ㄼ    ㄽ  ㄾ   ㄿ   ㅀ  ㅁ    ㅂ        ㅄ   ㅅ  ㅆ   ㅇ   ㅈ         ㅊ   ㅋ   ㅌ  ㅍ  ㅎ	

	//	ㅏ,ㅐ,ㅑ,	ㅒ,ㅓ,ㅔ,	ㅕ,ㅖ,ㅗ,ㅘ,ㅙ,ㅚ,ㅛ,ㅜ,ㅝ,ㅞ,ㅟ,ㅠ,ㅡ,ㅢ,ㅣ
*/
    
    final static int[] e2h_map = 
    {16,47,25,22,6, 8,29,38,32,34,30,50,48,43,31,35,17,0, 3,20,36,28,23,27,42,26,
     16,47,25,22,7, 8,29,38,32,34,30,50,48,43,33,37,18,1, 3,21,36,28,24,27,42,26};
  /*
//	 ㅁ, ㅠ,ㅊ, ㅇ,ㄷ,ㄹ,ㅎ,ㅗ, ㅑ,ㅓ, ㅏ,ㅣ,ㅡ,ㅜ, ㅐ,ㅔ, ㅂ,ㄱ,ㄴ,ㅅ,ㅕ,ㅍ, ㅈ,ㅌ,ㅛ, ㅋ,
	{16,47,25, 22,6, 8,29,38, 32,34, 30,50,48,43,31,35, 17,0, 3, 20,36,28, 23,27,42,26,
//	 ㅁ, ㅠ,ㅊ, ㅇ,ㄸ,ㄹ,ㅎ,ㅗ, ㅑ,ㅓ, ㅏ,ㅣ,ㅡ,ㅜ, ㅒ,ㅖ, ㅃ,ㄲ,ㄴ,ㅆ,ㅕ,ㅍ, ㅉ,ㅌ,ㅛ, ㅋ
	 16,47,25, 22,7, 8,29,38, 32,34, 30,50,48,43,33,37, 18,1, 3, 21,36,28, 24,27,42,26}; 
   */ 
/*
	final static char[] h_key_type =
//		0						     1							  2	
//		0, 1, 2, 3, 4, 5, 6, 7,8, 9, 0, 1, 2, 3, 4, 5, 6,7, 8, 9, 0, 1, 2, 3, 4,5, 6,7, 8, 9,
//		ㄱ,ㄲ,ㄳ,ㄴ,ㄵ,ㄶ,ㄷ,ㄸ,ㄹ,ㄺ,ㄻ,ㄼ,ㄽ,ㄾ,ㄿ,ㅀ,ㅁ,ㅂ,ㅃ,ㅄ,ㅅ,ㅆ, o,ㅈ,ㅉ,ㅊ,ㅋ,ㅌ,ㅍ,ㅎ,
	    { };
//		3						     4		
//		0, 1, 2, 3, 4, 5,6, 7, 8, 9, 0, 1, 2, 3,4, 5, 6, 7, 8, 9, 0		
//		ㅏ,ㅐ,ㅑ,ㅒ,ㅓ,ㅔ,ㅕ,ㅖ,ㅗ,ㅘ,ㅙ,ㅚ,ㅛ,ㅜ,ㅝ,ㅞ,ㅟ,ㅠ,ㅡ,ㅢ,ㅣ

	0x3131,ㄱ
	0x314F,ㅏ 
*/    

    
    private void clearHangul() {
        mHCursorState = HCURSOR_NONE;
        mHangulState = 0;
        previousHangulCurPos = -1;
        mHangulKeyStack[0] = 0; 
        mHangulKeyStack[1] = 0;
        mHangulKeyStack[2] = 0;
        mHangulKeyStack[3] = 0;
        mHangulKeyStack[4] = 0;
        mHangulKeyStack[5] = 0;
        mHangulJamoStack[0] = 0;
        mHangulJamoStack[1] = 0;
        mHangulJamoStack[2] = 0;
        return;
    }
    
    private void hangulSendKey(int newHangulChar, int hCursor) {

		if (hCursor == HCURSOR_NEW) {
			 Log.i("Hangul", "HCURSOR_NEW");
			
			mComposing.append((char)newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_NEW;
		}
		else if (hCursor == HCURSOR_ADD) {
			mHCursorState = HCURSOR_ADD;
			 Log.i("Hangul", "HCURSOR_ADD");
	        if (mComposing.length() > 0) {
	            mComposing.setLength(0);
	        	getCurrentInputConnection().finishComposingText();	        	
	        }			
	        
	        mComposing.append((char)newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
		}
		else if (hCursor == HCURSOR_UPDATE) {
			 Log.i("Hangul", "HCURSOR_UPDATE");
			mComposing.setCharAt(0, (char)newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_UPDATE;
		}
		else if (hCursor == HCURSOR_APPEND) {
			 Log.i("Hangul", "HCURSOR_APPEND");			
			mComposing.append((char)newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_APPEND;
		}
		else if (hCursor == HCURSOR_NONE) {
			if (newHangulChar == -1) {
				Log.i("Hangul", "HCURSOR_NONE [DEL -1]");
				keyDownUp(KeyEvent.KEYCODE_DEL);
				clearHangul();				
			}
			else if (newHangulChar == -2) {
		        int hangulKeyIdx;
		        int cho_idx,jung_idx,jong_idx;

				 Log.i("Hangul", "HCURSOR_NONE [DEL -2]");
		        
				switch(mHangulState) {
				case H_STATE_0:
					keyDownUp(KeyEvent.KEYCODE_DEL);
		            break;
				case H_STATE_1: // 초성
//					keyDownUp(KeyEvent.KEYCODE_DEL);
		            mComposing.setLength(0);
		            getCurrentInputConnection().commitText("", 0);
					clearHangul();
					mHangulState = H_STATE_0;
					break;
				case H_STATE_2: // 초성(복자음)
	                newHangulChar = 0x3131 + mHangulKeyStack[0];
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                mHangulKeyStack[1] = 0;
	                mHangulJamoStack[0] = mHangulKeyStack[0];
	                mHangulState = H_STATE_1; // goto 초성		
	                break;
				case H_STATE_3: // 중성(단모음,복모음)
					if (mHangulKeyStack[3] == 0) {
//						keyDownUp(KeyEvent.KEYCODE_DEL);
			            mComposing.setLength(0);
			            getCurrentInputConnection().commitText("", 0);
						clearHangul();	
						mHangulState = H_STATE_0;
					}
					else {
						mHangulKeyStack[3] = 0;
		                newHangulChar = 0x314F + (mHangulKeyStack[2] - 30);
		                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
		                mHangulJamoStack[1] = mHangulKeyStack[2];
		                mHangulState = H_STATE_3; // goto 중성												
					}
					break;
				case H_STATE_4: // 초성,중성(단모음,복모음)
					if (mHangulKeyStack[3] == 0) {
						mHangulKeyStack[2] = 0;
						mHangulJamoStack[1] = 0;
		                newHangulChar = 0x3131 + mHangulJamoStack[0];
		                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
		                mHangulState = H_STATE_1; // goto 초성						
					}
					else {
						mHangulJamoStack[1]= mHangulKeyStack[2];
						mHangulKeyStack[3] = 0;
		                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
		                jung_idx = mHangulJamoStack[1] - 30;
		                jong_idx = h_jongsung_idx[mHangulJamoStack[2]];
		                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
		                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
					}
					break;
				case H_STATE_5:	// 초성,중성,종성
					mHangulJamoStack[2] = 0;
					mHangulKeyStack[4] = 0;
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = h_jongsung_idx[mHangulJamoStack[2]];
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                mHangulState = H_STATE_4;
					break;
				case H_STATE_6:		
                    mHangulKeyStack[5] = 0;
                    mHangulJamoStack[2] = mHangulKeyStack[4];
                    cho_idx = h_chosung_idx[mHangulJamoStack[0]];
                    jung_idx = mHangulJamoStack[1] - 30;
                    jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];;
                    newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
                    mHangulState = H_STATE_5;
					break;
				}
			}
			else if (newHangulChar == -3) {
				 Log.i("Hangul", "HCURSOR_NONE [DEL -3]");				
		        final int length = mComposing.length();
		        if (length > 1) {
		            mComposing.delete(length - 1, length);				
		        }
			}
			
		}    	
    }   

    private int prevJaumKeyCode = 0;
    
        
	final private char ko_first_state[] = {
			1,3,4,6,7,8,10,12,13,	// 0
			16,3,4,6,7,8,10,12,13,	// 1
			1,3,4,6,7,8,10,12,13,	// 2 
			1,3,4,6,7,8,10,12,13,	// 3
			1,3,17,6,7,8,10,12,13,	// 4
			1,3,4,6,7,8,10,12,13,	// 5
			1,3,4,6,7,8,10,12,13,	// 6
			1,3,4,6,7,8,10,12,13,	// 7
			1,3,4,6,7,18,10,12,13,	// 8
			1,3,4,6,7,8,10,12,13,	// 9
			1,3,4,6,7,8,11,12,13,	// 10
			1,3,4,6,7,8,10,12,13,	// 11
			1,3,4,6,7,8,10,19,13,	// 12
			1,3,4,6,7,8,10,12,15,	// 13
			1,3,4,6,7,8,10,12,13,	// 14
			1,3,4,6,7,8,10,12,14,	// 15
			2,3,4,6,7,8,10,12,13,	// 16
			1,3,5,6,7,8,10,12,13,	// 17
			1,3,4,6,7,9,10,12,13,	// 18
			1,3,4,6,7,8,10,12,13,	// 19
		};	
	
	final private char ko_middle_state[] = {
//		 	0			   1			     2		   	3			    4			    5		    	6			    7				
			23,1,21,	7,2,11,		9,1,15,		4,5,0, 		0,0,0, 		6,3,0, 		0,0,0,		8,0,0,
//		 	8			    9		     	10		   	11			  12		   	13			  14		   	15
			0,0,0,		10,0,0,		0,0,0,		14,0,0,		13,0,0,		0,0,0,		0,12,0,		0,0,0,
//			16			  17			  18			  19			  20			  21			  22		  	23
			19,20,0,	18,0,0,		0,0,0,		0,0,0,		17,16,0,	22,16,0,	0,0,0,		0,3,0
		};	
	
	final private char ko_last_state[] = {
			1,4,7,8,16,17,19,21,22,	// 0
			24,0,0,0,0,0,3,0,0,		// 1
			1,0,0,0,0,0,0,0,0,		// 2	
			0,0,0,0,0,0,31,0,0,		// 3
			0,0,0,0,0,0,0,32,5,		// 4
			0,0,0,0,0,0,0,0,33,		// 5
			0,0,0,0,0,0,0,0,0,		// 6
			0,0,25,0,0,0,0,0,0,		// 7
			9,0,44,0,0,11,12,43,0,		// 8
			36,0,0,0,0,0,0,0,0,		// 9
			0,0,0,0,0,0,0,0,0,		// 10
			0,0,0,0,0,14,0,0,0,		// 11
			0,0,0,0,0,0,39,0,0,		// 12
			0,0,45,0,0,0,0,0,0,		// 13
			0,0,0,0,0,38,0,0,0,		// 14
			0,0,0,0,0,0,0,43,0,		// 15
			0,0,0,0,0,0,0,0,0,		// 16
			0,0,0,0,0,26,18,0,0,		// 17
			0,0,0,0,0,0,41,0,0,		// 18
			0,0,0,0,0,0,20,0,0,		// 19
			0,0,0,0,0,0,19,0,0,		// 20
			0,0,0,0,0,0,0,0,0,		// 21
			0,0,0,0,0,0,0,0,23,		// 22
			0,0,0,0,0,0,0,0,42,		// 23
			2,0,0,0,0,0,0,0,0,		// 24
			0,0,28,0,0,0,0,0,0,		// 25
			0,0,0,0,0,29,0,0,0,		// 26
			0,0,0,0,0,0,0,0,0,		// 27
			0,0,7,0,0,0,0,0,0,		// 28
			0,0,0,0,0,17,0,0,0,		// 29
			0,0,0,0,0,0,0,0,0,		// 30
			0,0,0,0,0,0,3,0,0,		// 31
			0,0,0,0,0,0,0,35,0,		// 32
			0,0,0,0,0,0,0,0,34,		// 33
			0,0,0,0,0,0,0,0,5,		// 34
			0,0,0,0,0,0,0,32,0,		// 35
			37,0,0,0,0,0,0,0,0,		// 36
			9,0,0,0,0,0,0,0,0,		// 37
			0,0,0,0,0,11,0,0,0,		// 38
			0,0,0,0,0,0,12,0,0,		// 39
			0,0,0,0,0,0,0,0,0,		// 40
			0,0,0,0,0,0,18,0,0,		// 41
			0,0,0,0,0,0,0,0,22,		// 42
			0,0,0,0,0,0,0,15,0,		// 43
			0,0,13,0,0,0,0,0,0,		// 44
			0,0,44,0,0,0,0,0,0,		// 45
		};	

	final private char ko_jong_m_split[] = {
			0,1, 0,2, 1,10, 0,3, 4,13,
			4,19, 0,4, 0,6, 8,1, 8,7,
			8,8, 8,10, 8,17, 8,18, 8,19,
			0,7,0,8,17,10,0,10,0,11,
			0,12,0,13,0,15,0,16,0,17,
			0,18,0,19
		};
	
	final private char ko_jong_l_split[] = {
			0,5,0,9,
			1,19,1,11,
			4,12,4,15,4,14,4,19,
			8,16,8,2,8,9,8,11,
			17,19,17,11,
			0,14,
			8,12,8,4,8,5
		};
	
	final private char jongsung_28idx[] = {
			0, 0, 1, 1, 4, 4, 4, 8, 8, 8, 8, 17, 17, 0, 8, 8, 8 
		};	
	
	final static char KO_S_0000 = 0;
	final static char KO_S_0100 = 1;
	final static char KO_S_1000 = 2;
	final static char KO_S_1100 = 3;
	final static char KO_S_1110 = 4;
	final static char KO_S_1111 = 5;

    private void clearSejong() {
    	ko_state_idx = KO_S_0000;
    	ko_state_first_idx = 0;
    	ko_state_middle_idx = 0;
    	ko_state_last_idx = 0;
    	ko_state_next_idx = 0;
    	prev_key = -1;
        return;
    }
    
    private int prev_key = -1;
	private char ko_state_idx = KO_S_0000;
	private char ko_state_first_idx; 
	private char ko_state_middle_idx;
	private char ko_state_last_idx;
	private char ko_state_next_idx;

	
    final private int key_idx[] =   
    	  {0, 1, 2, 3, 4, 5, 6, 7, 8,0,1,2,};
    	// ㄱ,ㄴ,ㄷ,ㄹ,ㅁ,ㅂ,ㅅ, ㅇ,ㅈ,ㅣ,ㆍ,ㅡ,
    

    final private char chosung_code[] = {
    		0, 1, 3, 6, 7, 8, 16, 17, 18, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
    	};
    final private char jongsung_code[] = {
    		0,1,2,3,4,5,6,  // ㄸ
    		8,9,10,11,12,13,14,15,16,17, // ㅃ
    		19,20,21,22,23, // ㅉ
    		25,26,27,28,29
    };
    
    final private char jungsung_stack[] = {
    	// 1  2 3  4  5  6  7 8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23	
    	// . .. ㅏ,ㅐ,ㅑ,ㅒ,ㅓ,ㅔ,ㅕ,ㅖ,ㅗ,ㅘ,ㅙ, ㅚ,ㅛ,ㅜ,ㅝ,ㅞ,ㅟ,ㅠ,ㅡ,ㅢ, ㅣ
    	   0,0, 0, 0, 0, 0, 0, 0,0, 0, 0, 11,11,11, 0, 0,16,16,16,0, 0, 21, 0 	
    };
    
    private void sendSejongKey(char newHangulChar, char hCursor) {

    	Log.i("Hangul", "newHangulChar[" + Integer.toString(newHangulChar) + "]");
    	
		if (hCursor == HCURSOR_NEW) {
			 Log.i("Hangul", "HCURSOR_NEW");
			
			mComposing.append(newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_NEW;
		}
		else if (hCursor == HCURSOR_ADD) {
			mHCursorState = HCURSOR_ADD;
			 Log.i("Hangul", "HCURSOR_ADD");
	        if (mComposing.length() > 0) {
	            mComposing.setLength(0);
	        	getCurrentInputConnection().finishComposingText();	        	
	        }			
	        
	        mComposing.append(newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
		}
		else if (hCursor == HCURSOR_UPDATE) {
			 Log.i("Hangul", "HCURSOR_UPDATE");
			mComposing.setCharAt(0, newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_UPDATE;
		}
		else if (hCursor == HCURSOR_APPEND) {
			 Log.i("Hangul", "HCURSOR_APPEND");			
			mComposing.append(newHangulChar);
			getCurrentInputConnection().setComposingText(mComposing, 1);
			mHCursorState = HCURSOR_APPEND;
		}
		else if (hCursor == HCURSOR_UPDATE_LAST) {
			 Log.i("Hangul", "HCURSOR_UPDATE_LAST");
				mComposing.setCharAt(1, newHangulChar);
				getCurrentInputConnection().setComposingText(mComposing, 1);
				mHCursorState = HCURSOR_UPDATE_LAST;	
		}
		else if (hCursor == HCURSOR_DELETE_LAST) {
			 Log.i("Hangul", "HCURSOR_DELETE_LAST");				
		     final int length = mComposing.length();
		     if (length > 1) {
		    	 Log.i("Hangul", "Delete start :" + Integer.toString(length));
	            mComposing.delete(length - 1, length);	
	            getCurrentInputConnection().setComposingText(mComposing, 1);
			}			
		}
		else if (hCursor == HCURSOR_DELETE) {
			char hChar;
	        char cho_idx, jung_idx, jong_idx;
        	switch(ko_state_idx) {        	
        	case KO_S_0000: 
        	case KO_S_0100:
        	case KO_S_1000:
				keyDownUp(KeyEvent.KEYCODE_DEL);
				clearHangul();        		
        		break;
        	case KO_S_1100:
        		ko_state_middle_idx = jungsung_stack[ko_state_middle_idx - 1];
        		if (ko_state_middle_idx > 0) {
    				cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
                	jung_idx = (char)(ko_state_middle_idx - 3);
                	hChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
                    sendSejongKey(hChar, HCURSOR_UPDATE);        			
        		}
        		else {
                    newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
                    sendSejongKey(newHangulChar, HCURSOR_UPDATE);				
    				ko_state_idx = KO_S_1000;        		        			
        		}
        		break;
        	case KO_S_1110:
				cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
            	jung_idx = (char)(ko_state_middle_idx - 3);
            	hChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
                sendSejongKey(hChar, HCURSOR_UPDATE);
                ko_state_last_idx = 0;
				ko_state_idx = KO_S_1110;        		        		                
        		break;
        	case KO_S_1111:
				ko_state_last_idx = jongsung_28idx[ko_state_last_idx - 28];
				sendSejongKey((char)0,HCURSOR_DELETE_LAST);
				ko_state_next_idx = 0;
				ko_state_idx = KO_S_1110;        		        		
        		break;
        	}			
		}
		 
    }   
    
    private char getJungsungCode(char jungsung_idx)
    {
    	Log.i("Hangul", "getJungsungCode[" + Integer.toString(jungsung_idx) + "]");
    	
    	switch(jungsung_idx) {
    	case 1:
    		return 0xB7; // .
    		// return 0x318D; .
    	case 2:
    		return 0x3A; // :
    		// return 0x2025; // .. 
    	default :
    		return  (char)(0x314F + jungsung_idx - 3);
    	}
    }
    
	private void handleSejong(int primaryCode, int[] keyCodes) {

		char new_last_idx;
        int base_idx;
        char new_state_idx;
        int idx;
        char newHangulChar;
        char cho_idx, jung_idx, jong_idx;
        int key = primaryCode;
        
        base_idx = primaryCode - 0x41;
		idx = key_idx[base_idx];

		Log.i("Hangul", "state[" + Integer.toString(ko_state_idx) + "]" + "["
				+ Integer.toString(ko_state_first_idx) + ","
				+ Integer.toString(ko_state_middle_idx) + ","
				+ Integer.toString(ko_state_last_idx) + ","
				+ Integer.toString(ko_state_next_idx) + "]" 
				);
		Log.i("Hangul", "base_idx,idx[" + Integer.toString(base_idx) +
									":" + Integer.toString(idx) + "]");
        if (base_idx < 9) {
        	switch(ko_state_idx) {        	
        	case KO_S_0000: // clear
				ko_state_first_idx = ko_first_state[ko_state_first_idx * 9 + idx];
//				sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);
                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
                sendSejongKey(newHangulChar, HCURSOR_NEW);				
				ko_state_idx = KO_S_1000;        		
        		break;
        	case KO_S_0100: // 중성
				ko_state_first_idx = ko_first_state[ko_state_first_idx * 9 + idx];
				ko_state_middle_idx = 0;
//				sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);
                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
                sendSejongKey(newHangulChar, HCURSOR_NEW);				
				ko_state_idx = KO_S_1000;     		
        		break;
        	case KO_S_1000: // 초성
				if (key == prev_key) {
					new_state_idx = ko_first_state[ko_state_first_idx * 9 + idx];
					if (new_state_idx == ko_state_first_idx) {
		                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
		                sendSejongKey(newHangulChar, HCURSOR_ADD);				
//						sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);						
					}
					else {
						ko_state_first_idx = new_state_idx;
		                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
		                sendSejongKey(newHangulChar, HCURSOR_UPDATE);				
//						sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_UPDATE);
					}
				}
				else {
					// cursor_pos++;
					// cursor_len = 0;
					ko_state_first_idx = ko_first_state[idx];
//					sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);
	                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				
				}
        		break;
        	case KO_S_1100: // 초성,중성
				ko_state_last_idx = ko_last_state[ko_state_last_idx * 9 + idx];
				Log.i("Hangul","ko_state_last_idx[" + Integer.toString(ko_state_last_idx) + "]");
//				sendSejongKey((char)(0x3131 + jongsung_code[ko_state_last_idx - 1]) ,HCURSOR_ADD);
				if (ko_state_middle_idx > 2) {
	                cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                jong_idx = h_jongsung_idx[jongsung_code[ko_state_last_idx - 1]+1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);
					ko_state_idx = KO_S_1110;        		
				}
				else {
					Log.i("Hangul", "Not Combination...");
					// must be implemented.
				}
        		break;
        	case KO_S_1110:
				new_last_idx = ko_last_state[ko_state_last_idx * 9 + idx];
				
				if(new_last_idx >= 28) {
					ko_state_last_idx = new_last_idx;
					ko_state_next_idx 
						= ko_jong_l_split[(new_last_idx - 28) * 2 + 1];

//					sendSejongKey((char)(0x3131 + chosung_code[ko_state_next_idx - 1]),HCURSOR_ADD);

					cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                char split_last_idx = jongsung_28idx[ko_state_last_idx - 28];
	                Log.i("Hangul", "split_last_idx[" + Integer.toString(split_last_idx) + "]");
	                if (split_last_idx > 0) {
//	 	               Log.i("Hangul", "jongsung_code[" + Integer.toString(jongsung_code[split_last_idx-1]) + "]");
//	    	            Log.i("Hangul", "jong_idx[" + Integer.toString(h_jongsung_idx[jongsung_code[split_last_idx - 1]+1]) + "]");
		                jong_idx = h_jongsung_idx[jongsung_code[split_last_idx - 1]+1];
		                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
	                }
	                else {
	                	newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
	                }
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);

					newHangulChar = (char)(0x3131 + chosung_code[ko_state_next_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_APPEND);
	                									
					ko_state_idx = KO_S_1111;
				}
				else if (new_last_idx == 0) {

					// cursor_pos++;
					// cursor_len = 0;

					ko_state_first_idx = 0;
					ko_state_middle_idx = 0;
					ko_state_last_idx = 0;

					idx = key_idx[base_idx];
					ko_state_first_idx = ko_first_state[ko_state_first_idx * 9 + idx];
//					sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);
	                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_ADD);									
					ko_state_idx = KO_S_1000;
				}
				else {
					ko_state_last_idx = new_last_idx;					
//					sendSejongKey((char)(0x3131 + jongsung_code[ko_state_last_idx - 1]) ,HCURSOR_UPDATE);
	                cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                jong_idx = h_jongsung_idx[jongsung_code[ko_state_last_idx - 1]+1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);
				}
        		
        		break;
        	case KO_S_1111:
				new_last_idx = ko_last_state[ko_state_last_idx * 9 + idx];
				
				if(new_last_idx >= 28) {
					ko_state_next_idx 
						= ko_jong_l_split[(new_last_idx - 28) * 2 + 1];
//					sendSejongKey((char)(0x3131 + chosung_code[ko_state_next_idx - 1]),HCURSOR_UPDATE);
					ko_state_last_idx = new_last_idx;

					cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                char split_last_idx = jongsung_28idx[ko_state_last_idx - 28];
	                jong_idx = h_jongsung_idx[jongsung_code[split_last_idx - 1]+1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);

					newHangulChar = (char)(0x3131 + chosung_code[ko_state_next_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE_LAST);
					
					ko_state_idx = KO_S_1111;
				}
				else {
					if (prev_key == key) {
						ko_state_last_idx = new_last_idx;
						// delete last cursor
//						sendSejongKey((char)(0x3131 + jongsung_code[ko_state_last_idx - 1]) ,HCURSOR_UPDATE);
						ko_state_next_idx = 0;

						cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
		                jung_idx = (char)(ko_state_middle_idx - 3);
		                jong_idx = h_jongsung_idx[jongsung_code[ko_state_last_idx - 1]+1];
		                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
		                sendSejongKey(newHangulChar, HCURSOR_UPDATE);
						
		                sendSejongKey((char)0,HCURSOR_DELETE_LAST);
						ko_state_idx = KO_S_1110;
					}
					else { 
						clearSejong();
						ko_state_first_idx = ko_first_state[ko_state_first_idx * 9 + idx];
//						sendSejongKey((char)(0x3131 + chosung_code[ko_state_first_idx - 1]),HCURSOR_ADD);
		                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
		                sendSejongKey(newHangulChar, HCURSOR_ADD);										
						ko_state_idx = KO_S_1000;
					}
				}        		
        		break;
        	
        	}
        }
        else {        	
			switch(ko_state_idx) {
        	
        	case KO_S_0000:
        		/*
				if (ko_middle_state[ko_state_middle_idx * 3 + idx] == 0) {
					ko_state_middle_idx = 0;
				}
				*/
				ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
//				sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_ADD);
                newHangulChar = getJungsungCode(ko_state_middle_idx);
                sendSejongKey(newHangulChar, HCURSOR_NEW);				
				ko_state_idx = KO_S_0100;        		
        		break;
        	case KO_S_0100:
				if (ko_middle_state[ko_state_middle_idx * 3 + idx] == 0) {
					// cursor_pos++;
					// cursor_len = 0;
					ko_state_middle_idx = 0;
					ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
//					sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_ADD);
	                newHangulChar = getJungsungCode(ko_state_middle_idx);					
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				
				}
				else {
					ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
//					sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_UPDATE);
	                newHangulChar = getJungsungCode(ko_state_middle_idx);					
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);				
				}        		
        		break;
        	case KO_S_1000:
        		/*
				if (ko_middle_state[ko_state_middle_idx * 3 + idx] == 0) {
					ko_state_middle_idx = 0;
				}
				*/
				ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
//				sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_ADD);
				
				if (ko_state_middle_idx > 2) {
					cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
                	jung_idx = (char)(ko_state_middle_idx - 3);
//                	jong_idx = jongsung_code[ko_state_last_idx - 1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);				
                }
                else {
                    newHangulChar = getJungsungCode(ko_state_middle_idx);
                    sendSejongKey(newHangulChar, HCURSOR_APPEND);				
					// must be implemented.                	
                }
				ko_state_idx = KO_S_1100;
        		break;
        	case KO_S_1100:
				if (ko_middle_state[ko_state_middle_idx * 3 + idx] == 0) {
					// cursor_pos++;
					// cursor_len = 0;
					ko_state_first_idx = 0;
					ko_state_middle_idx = 0;
					ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
//					sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_ADD);
	                newHangulChar = getJungsungCode(ko_state_middle_idx);					
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				
					ko_state_idx = KO_S_0100;
				}
				else {
					ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
					if (ko_state_middle_idx > 2) {
	//					sendSejongKey(getJungsungCode(ko_state_middle_idx),HCURSOR_UPDATE);
		                cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
		                jung_idx = (char)(ko_state_middle_idx - 3);
	//	                jong_idx = jongsung_code[ko_state_last_idx - 1];
		                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
		                sendSejongKey(newHangulChar, HCURSOR_UPDATE);
		                sendSejongKey((char)0,HCURSOR_DELETE_LAST);
					}
					else {
	                    newHangulChar = getJungsungCode(ko_state_middle_idx);
	                    sendSejongKey(newHangulChar, HCURSOR_UPDATE_LAST);										
					}
				}
        		
        		break;
        	case KO_S_1110:
				if (ko_jong_m_split[(ko_state_last_idx - 1) * 2] > 0) {
					// update jongsong 
//					sendSejongKey((char)(0x3131 + ko_jong_m_split[(ko_state_last_idx - 1) * 2]) ,HCURSOR_UPDATE);
					cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                jong_idx = h_jongsung_idx[jongsung_code[ko_jong_m_split[(ko_state_last_idx - 1) * 2]]];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);


					// xc_input_automata_push(1,	ko_state_first_idx - 1, 
					//						ko_state_middle_idx - 1,
					//						ko_jong_m_split[(ko_state_last_idx - 1) * 2],-1);

				}
				else {
					// xc_input_automata_push(1,ko_state_first_idx - 1, 
					//						ko_state_middle_idx - 1,-1,-1);
					/// delete jongsong
					cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
	                jung_idx = (char)(ko_state_middle_idx - 3);
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
	                sendSejongKey(newHangulChar, HCURSOR_UPDATE);
				}

				// cursor_pos++;
				// cursor_len = 0;

				ko_state_first_idx 
					= ko_jong_m_split[(ko_state_last_idx - 1) * 2 + 1];
				ko_state_middle_idx = 0;
				ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];			
                
                if (ko_state_middle_idx > 2) {
    				cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
                    jung_idx = (char)(ko_state_middle_idx - 3);
//                  jong_idx = jongsung_code[ko_state_last_idx - 1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				
					ko_state_idx = KO_S_1100;
                }
                else {
	                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				                	
                    newHangulChar = getJungsungCode(ko_state_middle_idx);
                    sendSejongKey(newHangulChar, HCURSOR_APPEND);				
					// must be implemented.                	
                }

				ko_state_last_idx = 0;
				ko_state_idx = KO_S_1100;        		
        		break;
        	case KO_S_1111:
				if (ko_state_last_idx >= 28) {
					ko_state_last_idx = jongsung_28idx[ko_state_last_idx - 28];
				}
				// xc_input_automata_push(1, 	ko_state_first_idx - 1, 
				//						ko_state_middle_idx - 1,
				//						ko_state_last_idx,-1);
				// cursor_pos++;
				// cursor_len = 0;

				ko_state_first_idx = ko_state_next_idx;								
				ko_state_middle_idx = 0;
				ko_state_middle_idx = ko_middle_state[ko_state_middle_idx * 3 + idx];
				
				sendSejongKey((char)0,HCURSOR_DELETE_LAST);
				
                if (ko_state_middle_idx > 2) {
    				cho_idx = h_chosung_idx[chosung_code[ko_state_first_idx - 1]];
                    jung_idx = (char)(ko_state_middle_idx - 3);
//                  jong_idx = jongsung_code[ko_state_last_idx - 1];
	                newHangulChar = (char)(0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28)));
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				
					ko_state_idx = KO_S_1100;
                }
                else {
	                newHangulChar = (char)(0x3131 + chosung_code[ko_state_first_idx - 1]);
	                sendSejongKey(newHangulChar, HCURSOR_ADD);				                	
                    newHangulChar = getJungsungCode(ko_state_middle_idx);
                    sendSejongKey(newHangulChar, HCURSOR_APPEND);				
                }

                ko_state_last_idx = 0;
				ko_state_next_idx = 0;
				ko_state_idx = KO_S_1100;        		
        		break;
        	
        	}        	
        }
        
//        sendSeongKey();
        
        prev_key = key;
    }
    

    private void handleHangul(int primaryCode, int[] keyCodes) {

        int hangulKeyIdx = -1;
        int newHangulChar;
        int cho_idx,jung_idx,jong_idx;
        int hangulChar = 0;
/*        
        if (mHangulCursorMoved == 1) {
        	clearHangul();
        	Log.i("Hangul", "clear Hangul at handleHangul by mHangulCursorMoved");
        	mHangulCursorMoved = 0;
        }
*/        
       	// Log.i("Hangul", "PrimaryCode[" + Integer.toString(primaryCode)+"]"); 

        if (primaryCode >= 0x61 && primaryCode <= 0x7A) {
    //            Log.i("SoftKey", "handleHangul - hancode");

            if (mHangulShiftState == 0) {
            	hangulKeyIdx = e2h_map[primaryCode - 0x61];
            }
            else {
            	hangulKeyIdx = e2h_map[primaryCode - 0x61 + 26];   
//                Keyboard currentKeyboard = mInputView.getKeyboard();
            	mHangulShiftedKeyboard.setShifted(false);
                mInputView.setKeyboard(mHangulKeyboard);
                mHangulKeyboard.setShifted(false);
                mHangulShiftState = 0;                           	
            }
            hangulChar = 1;
        }
        else if (primaryCode >= 0x41 && primaryCode <= 0x5A) {
        	hangulKeyIdx = e2h_map[primaryCode - 0x41 + 26];   
        	hangulChar = 1;
        }
        /*
        else  if (primaryCode >= 0x3131 && primaryCode <= 0x3163) {
        	hangulKeyIdx = primaryCode - 0x3131;
        	hangulChar = 1;
        }
        */
        else {
        	hangulChar = 0;
        }
        
        
        if (hangulChar == 1) {
        	
	        switch(mHangulState) {
	
	        case H_STATE_0: // Hangul Clear State
	            // Log.i("SoftKey", "HAN_STATE 0");
	            if (hangulKeyIdx < 30) { // if 자음
	                newHangulChar = 0x3131 + hangulKeyIdx;
	                hangulSendKey(newHangulChar, HCURSOR_NEW);
	                mHangulKeyStack[0] = hangulKeyIdx; 
	                mHangulJamoStack[0] = hangulKeyIdx;
	                mHangulState = H_STATE_1; // goto 초성
	            }
	            else { // if 모음
	                newHangulChar = 0x314F + (hangulKeyIdx - 30);
	                hangulSendKey(newHangulChar, HCURSOR_NEW);
	                mHangulKeyStack[2] = hangulKeyIdx;
	                mHangulJamoStack[1] = hangulKeyIdx;
	                mHangulState = H_STATE_3; // goto 중성
	            }
	            break;
	
	        case H_STATE_1: // 초성
	            // Log.i("SoftKey", "HAN_STATE 1");
	            if (hangulKeyIdx < 30) { // if 자음
	                int newHangulKeyIdx = isHangulKey(0,hangulKeyIdx);
	                if (newHangulKeyIdx > 0) { // if 복자음
	                    newHangulChar = 0x3131 + newHangulKeyIdx;
	                    mHangulKeyStack[1] = hangulKeyIdx;
	                    mHangulJamoStack[0] = newHangulKeyIdx;
//	                    hangulSendKey(-1);
	                    hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                    mHangulState = H_STATE_2; // goto 초성(복자음)
	                }
	                else { // if 자음
	                	
	                	// cursor error trick start
		                newHangulChar = 0x3131 + mHangulJamoStack[0];
	                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
	                    // trick end
	                	
	                    newHangulChar = 0x3131 + hangulKeyIdx;
	                    hangulSendKey(newHangulChar, HCURSOR_ADD);
	                    mHangulKeyStack[0] = hangulKeyIdx;
	                    mHangulJamoStack[0] = hangulKeyIdx;
	                    mHangulState = H_STATE_1; // goto 초성
	                }
	            }
	            else { // if 모음
	                mHangulKeyStack[2] = hangulKeyIdx;
	                mHangulJamoStack[1] = hangulKeyIdx;
//	                hangulSendKey(-1);
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = h_jongsung_idx[mHangulJamoStack[2]];
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                mHangulState = H_STATE_4; // goto 초성,중성
	            }
	            break;
	
	        case H_STATE_2: // 초성(복자음)
	            // Log.i("SoftKey", "HAN_STATE 2");
	            if (hangulKeyIdx < 30) { // if 자음
	            	
                	// cursor error trick start
	                newHangulChar = 0x3131 + mHangulJamoStack[0];
                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
                    // trick end
	            	
	            	
	                mHangulKeyStack[0] = hangulKeyIdx;
	                mHangulJamoStack[0] = hangulKeyIdx;
	                mHangulJamoStack[1] = 0;
	                newHangulChar = 0x3131 + hangulKeyIdx;
	                hangulSendKey(newHangulChar, HCURSOR_ADD);
	                mHangulState = H_STATE_1; // goto 초성
	            }
	            else { // if 모음
	                newHangulChar = 0x3131 + mHangulKeyStack[0];
	                mHangulKeyStack[0] = mHangulKeyStack[1];
	                mHangulJamoStack[0] = mHangulKeyStack[0];
	                mHangulKeyStack[1] = 0;
//	                hangulSendKey(-1);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	
	                mHangulKeyStack[2] = hangulKeyIdx;
	                mHangulJamoStack[1] = mHangulKeyStack[2];
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = 0;
	
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_ADD);
	                mHangulState = H_STATE_4; // goto 초성,중성
	            }
	            break;
	            
	        case H_STATE_3: // 중성(단모음,복모음)
	            // Log.i("SoftKey", "HAN_STATE 3");
	            if (hangulKeyIdx < 30) { // 자음
	            	
                	// cursor error trick start
	                newHangulChar = 0x314F + (mHangulJamoStack[1] - 30);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
                    // trick end
	            	
	                newHangulChar = 0x3131 + hangulKeyIdx;
	                hangulSendKey(newHangulChar, HCURSOR_ADD);
	                mHangulKeyStack[0] = hangulKeyIdx;
	                mHangulJamoStack[0] = hangulKeyIdx;
	                mHangulJamoStack[1] = 0;
	                mHangulState = H_STATE_1; // goto 초성
	            }
	            else { // 모음
	            	if (mHangulKeyStack[3] == 0) {	            		
		                int newHangulKeyIdx = isHangulKey(2,hangulKeyIdx);
		                if (newHangulKeyIdx > 0) { // 복모음
	//	                	hangulSendKey(-1);
		                    newHangulChar = 0x314F + (newHangulKeyIdx - 30);
		                    hangulSendKey(newHangulChar, HCURSOR_UPDATE);
		                    mHangulKeyStack[3] = hangulKeyIdx;
		                    mHangulJamoStack[1] = newHangulKeyIdx;
		                }
		                else { // 모음
		                	
		                	// cursor error trick start
		                    newHangulChar = 0x314F + (mHangulJamoStack[1] - 30);
		                    hangulSendKey(newHangulChar, HCURSOR_UPDATE);
		                    // trick end
							
		                    newHangulChar = 0x314F + (hangulKeyIdx - 30);
		                    hangulSendKey(newHangulChar,HCURSOR_ADD);
		                    mHangulKeyStack[2] = hangulKeyIdx;
		                    mHangulJamoStack[1] = hangulKeyIdx;
		                }
	            	}
	            	else {
	            		
	                	// cursor error trick start
	                    newHangulChar = 0x314F + (mHangulJamoStack[1] - 30);
	                    hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                    // trick end
						
	                    newHangulChar = 0x314F + (hangulKeyIdx - 30);
	                    hangulSendKey(newHangulChar,HCURSOR_ADD);
	                    mHangulKeyStack[2] = hangulKeyIdx;
	                    mHangulJamoStack[1] = hangulKeyIdx;	            	
	                    mHangulKeyStack[3] = 0;
	            	}
	                mHangulState = H_STATE_3;
	            }
	            break;
	        case H_STATE_4: // 초성,중성(단모음,복모음)
	           // Log.i("SoftKey", "HAN_STATE 4");
	            if (hangulKeyIdx < 30) { // if 자음
	                mHangulKeyStack[4] = hangulKeyIdx;
	                mHangulJamoStack[2] = hangulKeyIdx;
//	                hangulSendKey(-1);
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	                if (jong_idx == 0) { // if 종성 is not valid ex, 라 + ㅉ
	                    mHangulKeyStack[0] = hangulKeyIdx;
	                    mHangulKeyStack[1] = 0;
	                    mHangulKeyStack[2] = 0;
	                    mHangulKeyStack[3] = 0;
	                    mHangulKeyStack[4] = 0;
	                    mHangulJamoStack[0] = hangulKeyIdx;
	                    mHangulJamoStack[1] = 0;
	                    mHangulJamoStack[2] = 0;
	                    newHangulChar = 0x3131 + hangulKeyIdx;
	                    hangulSendKey(newHangulChar,HCURSOR_ADD);		                
		                mHangulState = H_STATE_1; // goto 초성
	                }
	                else {
	                	mHangulState = H_STATE_5; // goto 초성,중성,종성
	                }
	            }
	            else { // if 모음
	            	if (mHangulKeyStack[3] == 0) {
		                int newHangulKeyIdx = isHangulKey(2,hangulKeyIdx);
		                if (newHangulKeyIdx > 0) { // if 복모음
	//	                	hangulSendKey(-1);
	//	                    mHangulKeyStack[2] = newHangulKeyIdx;
		                    mHangulKeyStack[3] = hangulKeyIdx;
		                    mHangulJamoStack[1] = newHangulKeyIdx;
		                    cho_idx = h_chosung_idx[mHangulJamoStack[0]];
		                    jung_idx = mHangulJamoStack[1] - 30;
		                    jong_idx = 0;
		                    newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
		                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
		                    mHangulState = H_STATE_4; // goto 초성,중성
		                }
		                else { // if invalid 복모음
		                	
		                	// cursor error trick start
			                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
			                jung_idx = mHangulJamoStack[1] - 30;
			                jong_idx = 0;		
			                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
		                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
		                    // trick end
		                	
		                    newHangulChar = 0x314F + (hangulKeyIdx - 30);
		                    hangulSendKey(newHangulChar,HCURSOR_ADD);
		                    mHangulKeyStack[0] = 0;
		                    mHangulKeyStack[1] = 0;
		                    mHangulJamoStack[0] = 0;
		                    mHangulKeyStack[2] = hangulKeyIdx;
		                    mHangulJamoStack[1] = hangulKeyIdx;
		                    mHangulState = H_STATE_3; // goto 중성
		                }
	            	}
	            	else {
	            		
	                	// cursor error trick start
		                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
		                jung_idx = mHangulJamoStack[1] - 30;
		                jong_idx = 0;		
		                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
	                    // trick end
	                	
	            		
	                    newHangulChar = 0x314F + (hangulKeyIdx - 30);
	                    hangulSendKey(newHangulChar,HCURSOR_ADD);
	                    mHangulKeyStack[0] = 0;
	                    mHangulKeyStack[1] = 0;
	                    mHangulJamoStack[0] = 0;
	                    mHangulKeyStack[2] = hangulKeyIdx;
	                    mHangulJamoStack[1] = hangulKeyIdx;
	                    mHangulKeyStack[3] = 0;
	                    mHangulState = H_STATE_3; // goto 중성
	            		
	            	}
	            }
	            break;
	        case H_STATE_5: // 초성,중성,종성
	            // Log.i("SoftKey", "HAN_STATE 5");
	            if (hangulKeyIdx < 30) { // if 자음
	                int newHangulKeyIdx = isHangulKey(4,hangulKeyIdx);
	                if (newHangulKeyIdx > 0) { // if 종성 == 복자음
//	                	hangulSendKey(-1);
	                    mHangulKeyStack[5] = hangulKeyIdx;
	                    mHangulJamoStack[2] = newHangulKeyIdx;
	
	                    cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                    jung_idx = mHangulJamoStack[1] - 30;
	                    jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];;
	                    newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
	                    mHangulState = H_STATE_6; // goto  초성,중성,종성(복자음)
	                }
	                else { // if 종성 != 복자음
	                	
	                	// cursor error trick start
	                    cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                    jung_idx = mHangulJamoStack[1] - 30;
	                    jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];;
	                    newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
	                    // trick end
	                    
	                	
	                    mHangulKeyStack[0] = hangulKeyIdx;
	                    mHangulKeyStack[1] = 0;
	                    mHangulKeyStack[2] = 0;
	                    mHangulKeyStack[3] = 0;
	                    mHangulKeyStack[4] = 0;
	                    mHangulJamoStack[0] = hangulKeyIdx;
	                    mHangulJamoStack[1] = 0;
	                    mHangulJamoStack[2] = 0;
	                    newHangulChar = 0x3131 + hangulKeyIdx;
	                    hangulSendKey(newHangulChar,HCURSOR_ADD);
	                    mHangulState = H_STATE_1; // goto 초성
	                }
	            }
	            else { // if 모음
//	            	hangulSendKey(-1);
	
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = 0;
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	
	                mHangulKeyStack[0] = mHangulKeyStack[4];
	                mHangulKeyStack[1] = 0;
	                mHangulKeyStack[2] = hangulKeyIdx;
	                mHangulKeyStack[3] = 0;
	                mHangulKeyStack[4] = 0;
	                mHangulJamoStack[0] = mHangulKeyStack[0];
	                mHangulJamoStack[1] = mHangulKeyStack[2];
	                mHangulJamoStack[2] = 0;
	
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = 0;
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_ADD);
	
	                // Log.i("SoftKey", "--- Goto HAN_STATE 4");
	                mHangulState = H_STATE_4; // goto 초성,중성
	            }
	            break;
	        case H_STATE_6: // 초성,중성,종성(복자음)
	            // Log.i("SoftKey", "HAN_STATE 6");
	            if (hangulKeyIdx < 30) { // if 자음
	            	
                	// cursor error trick start
                    cho_idx = h_chosung_idx[mHangulJamoStack[0]];
                    jung_idx = mHangulJamoStack[1] - 30;
                    jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];;
                    newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
                    hangulSendKey(newHangulChar,HCURSOR_UPDATE);
                    // trick end
	            	
	            	
	                mHangulKeyStack[0] = hangulKeyIdx;                
	                mHangulKeyStack[1] = 0;
	                mHangulKeyStack[2] = 0;
	                mHangulKeyStack[3] = 0;
	                mHangulKeyStack[4] = 0;
	                mHangulJamoStack[0] = hangulKeyIdx;
	                mHangulJamoStack[1] = 0;
	                mHangulJamoStack[2] = 0;
	
	                newHangulChar = 0x3131 + hangulKeyIdx;
	                hangulSendKey(newHangulChar,HCURSOR_ADD);
	
	                mHangulState = H_STATE_1; // goto 초성
	            }
	            else { // if 모음
//	            	hangulSendKey(-1);
	                mHangulJamoStack[2] = mHangulKeyStack[4];
	
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = h_jongsung_idx[mHangulJamoStack[2]+1];;
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar, HCURSOR_UPDATE);
	
	                mHangulKeyStack[0] = mHangulKeyStack[5];
	                mHangulKeyStack[1] = 0;
	                mHangulKeyStack[2] = hangulKeyIdx;
	                mHangulKeyStack[3] = 0;
	                mHangulKeyStack[4] = 0;
	                mHangulKeyStack[5] = 0;
	                mHangulJamoStack[0] = mHangulKeyStack[0];
	                mHangulJamoStack[1] = mHangulKeyStack[2];
	                mHangulJamoStack[2] = 0;
	
	                cho_idx = h_chosung_idx[mHangulJamoStack[0]];
	                jung_idx = mHangulJamoStack[1] - 30;
	                jong_idx = 0;
	                newHangulChar = 0xAC00 + ((cho_idx * 21 * 28) + (jung_idx * 28) + jong_idx);
	                hangulSendKey(newHangulChar,HCURSOR_ADD);
	
	                mHangulState = H_STATE_4; // goto 초성,중성
	            }
	            break;
	        } 
        }
        else {
            // Log.i("Hangul", "handleHangul - No hancode");
            clearHangul();
            sendKey(primaryCode);
        }

    }  
// Hangul Code End    
        
    private void handleCharacter(int primaryCode, int[] keyCodes) {
        if (isInputViewShown()) {
            if (mInputView.isShifted()) {
                primaryCode = Character.toUpperCase(primaryCode);
            }
        }
        if (isAlphabet(primaryCode) && mPredictionOn) {
            mComposing.append((char) primaryCode);
            getCurrentInputConnection().setComposingText(mComposing, 1);
            updateShiftKeyState(getCurrentInputEditorInfo());
            updateCandidates();
        } else {
        	sendKeyChar((char)primaryCode);
        	/*
            getCurrentInputConnection().commitText(
                    String.valueOf((char) primaryCode), 1);
            */
        }
    }

    private void handleClose() {
        commitTyped(getCurrentInputConnection());
        requestHideSelf(0);
        mInputView.closing();
    }

    private void checkToggleCapsLock() {
        long now = System.currentTimeMillis();
        if (mLastShiftTime + 800 > now) {
            mCapsLock = !mCapsLock;
            mLastShiftTime = 0;
        } else {
            mLastShiftTime = now;
        }
    }
    
    private String getWordSeparators() {
        return mWordSeparators;
    }
    
    public boolean isWordSeparator(int code) {
        String separators = getWordSeparators();
        return separators.contains(String.valueOf((char)code));
    }

    public void pickDefaultCandidate() {
        pickSuggestionManually(0);
    }
    
    public void pickSuggestionManually(int index) {
        if (mCompletionOn && mCompletions != null && index >= 0
                && index < mCompletions.length) {
            CompletionInfo ci = mCompletions[index];
            getCurrentInputConnection().commitCompletion(ci);
            if (mCandidateView != null) {
                mCandidateView.clear();
            }
            updateShiftKeyState(getCurrentInputEditorInfo());
        } else if (mComposing.length() > 0) {
            // If we were generating candidate suggestions for the current
            // text, we would commit one of them here.  But for this sample,
            // we will just commit the current text.
            commitTyped(getCurrentInputConnection());
        }
    }
    
    public void swipeRight() {
        if (mCompletionOn) {
            pickDefaultCandidate();
        }
    }
    
    public void swipeLeft() {
        handleBackspace();
    }

    public void swipeDown() {
        handleClose();
    }

    public void swipeUp() {
    }
    
    public void onPress(int primaryCode) {
    }
    
    public void onRelease(int primaryCode) {
    }
}
